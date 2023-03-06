#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/pic.h>
#include <cpu/smp.h>
#include <cpu/lapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/serial.h>
#include <sched/scheduler.h>
#include <subsys/socket.h>
#include <main/panic.h>
#include <misc/logger.h>

idt_descriptor_t idtr;
idt_gate_descriptor_t *gates;
extern void *int_table[];
void *redirectTable[256];
int redirectTableMeta[256];

// change gate information
void idtSetGate(void *handler, uint8_t entry, uint8_t attributes, bool user)
{
    idt_gate_descriptor_t *gate = &gates[entry]; // select the gate
    zero(gate, sizeof(idt_gate_descriptor_t));
    if (gate->segmentselector == 0)                 // detect if we didn't touch the gate
        idtr.size += sizeof(idt_gate_descriptor_t); // if we didn't we can safely increase the size

    gate->attributes = attributes;                                     // set the attributes
    gate->segmentselector = (8 * 1);                                   // set the kernel code selector from gdt
    gate->offset = (uint16_t)((uint64_t)handler & 0x000000000000ffff); // offset to the entry
    gate->offset2 = (uint16_t)(((uint64_t)handler & 0x00000000ffff0000) >> 16);
    gate->offset3 = (uint32_t)(((uint64_t)handler & 0xffffffff00000000) >> 32);

    // enable ists
    if (user)
        gate->ist = 2; // separate ists
    else
        gate->ist = 1;
}

// initialize the intrerupt descriptor table
void idtInit(uint16_t procID)
{
    cli(); // disable intrerrupts

    // allocate the gates
    gates = pmmPage();
    zero(gates, VMM_PAGE);

    idtr.offset = (uint64_t)gates; // set the offset to the data
    idtr.size = 0;                 // reset the size
    for (int i = 0; i < 0xFF; i++) // set all exception irqs to the base handler
        idtSetGate((void *)int_table[i], i, IDT_InterruptGateU, true);

    idtr.size--; // decrement to comply with the spec

    idtInstall(procID); // load the idtr

    // clear the redirection table
    zero(redirectTable, sizeof(redirectTable));

    logInfo("idt: loaded size %d", idtr.size);
}

void idtInstall(uint8_t procID)
{
    // setup ist
    gdt_tss_t *tss = tssGet()[procID];
    tss->ist[0] = (uint64_t)pmmPage() + VMM_PAGE;
    tss->ist[1] = (uint64_t)pmmPage() + VMM_PAGE;

    zero((void *)tss->ist[0] - VMM_PAGE, VMM_PAGE);
    zero((void *)tss->ist[1] - VMM_PAGE, VMM_PAGE);

    iasm("lidt %0" ::"m"(idtr)); // load the idtr and don't enable intrerrupts yet
}

void idtRedirect(void *handler, uint8_t entry, uint32_t tid)
{
    if (tid != redirectTableMeta[entry] && tid != 0 && redirectTableMeta[entry] != 0) // don't allow to overwrite another driver's redirect
        return;

    redirectTable[entry] = handler;
    redirectTableMeta[entry] = tid;
}

void idtClearRedirect(uint32_t tid)
{
    for (int i = 0; i < 256; i++)
        if (redirectTableMeta[i] == tid)
            redirectTable[i] = NULL;
}

extern void callWithPageTable(uint64_t rip, uint64_t pagetable);

const char *exceptions[] = {
    "Divide By Zero", "Debug", "NMI", "Breakpoint", "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available", "Double Fault", "_", "Invalid TSS", "Segment Not Present", "Stack Fault", "General Protection Fault", "Page Fault"};

void exceptionHandler(idt_intrerrupt_stack_t *stack, uint64_t int_num)
{
    vmmSwap(vmmGetBaseTable()); // swap to the base table

    if (redirectTable[int_num]) // there is a request to redirect intrerrupt to a driver
    {
        struct sched_task *task = schedulerGet(redirectTableMeta[int_num]);

        if (!task)
        {
            printks("the task doesn't exist anymore!\n");
            redirectTable[int_num] = NULL;
            goto cnt;
        }

        // printks("redirecting %x to %x (requested by task %d with stack %x)\n", int_num, redirectTable[int_num], redirectTableMeta[int_num], task->intrerruptStack.rsp);

        // this line gives the control to the driver
        callWithPageTable((uint64_t)redirectTable[int_num], (uint64_t)task->pageTable);

        goto cnt;
    }

cnt:
    if (int_num == APIC_TIMER_VECTOR)
        return lapicHandleTimer(stack);

    if (stack->cs == 0x23) // userspace
    {
        struct sock_socket *initSocket = sockGet(1);

        const char *name = schedulerGetCurrent()->name;

        logWarn("%s has crashed with %s! Terminating it.", name, exceptions[int_num]);

        if (initSocket)
        {
            char *str = pmmPage();
            zero(str, 6 + strlen(name));

            // construct a string based on the format "crash %s", name
            memcpy(str, "crash ", 6);
            memcpy(str + 6, name, strlen(name));

            sockAppend(initSocket, str, strlen(str)); // announce that the application has crashed

            pmmDeallocate(str);
        }

        schedulerKill(schedulerGetCurrent()->id); // terminate the task
        schedulerSchedule(stack);                 // schedule next task
        return;
    }

    framebufferClear(0);

    const char *message = to_hstring(int_num);

    if (int_num < sizeof(exceptions) / 8)
        message = exceptions[int_num];

    if (int_num == 0xE) // when a page fault occurs the faulting address is set in cr2
        logError("CR2=0x%p ", controlReadCR2());

    logError("CORE #%d: RIP=0x%p CS=0x%p RFLAGS=0x%p RSP=0x%p SS=0x%p ERR=0x%p", smpID(), stack->rip, stack->cs, stack->rflags, stack->rsp, stack->ss, stack->error);
    panick(message);
}