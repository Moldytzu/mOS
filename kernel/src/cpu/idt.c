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
    idt_gate_descriptor_t *gate = &gates[entry];    // select the gate
    if (gate->segmentselector == 0)                 // detect if we didn't touch the gate
        idtr.size += sizeof(idt_gate_descriptor_t); // if we didn't we can safely increase the size

    gate->attributes = attributes;                                     // set the attributes
    gate->segmentselector = (8 * 1);                                   // set the kernel code selector from gdt
    gate->offset = (uint16_t)((uint64_t)handler & 0x000000000000ffff); // offset to the entry
    gate->offset2 = (uint16_t)(((uint64_t)handler & 0x00000000ffff0000) >> 16);
    gate->offset3 = (uint32_t)(((uint64_t)handler & 0xffffffff00000000) >> 32);

    // enable ists
    if (entry == APIC_TIMER_VECTOR)
        gate->ist = 3;
    else if (user)
        gate->ist = 2;
    else
        gate->ist = 1;
}

void *idtGet()
{
    return (void *)gates;
}

// initialize the intrerupt descriptor table
void idtInit(uint16_t procID)
{
    cli(); // disable intrerrupts

    gates = pmmPage(); // allocate the gates

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
    tss->ist[2] = (uint64_t)pmmPage() + VMM_PAGE;

    tss->rsp[0] = (uint64_t)pmmPage() + VMM_PAGE;
    tss->rsp[1] = (uint64_t)pmmPage() + VMM_PAGE;
    tss->rsp[2] = (uint64_t)pmmPage() + VMM_PAGE;

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

    switch (int_num)
    {
    case APIC_NMI_VECTOR: // this halts the cpus in case of a kernel panic
        return hang();

    default:
        break;
    }

    if (redirectTable[int_num] && schedGet(redirectTableMeta[int_num])) // there is a request to redirect intrerrupt to a driver (todo: replace this with a struct)
    {
        callWithPageTable((uint64_t)redirectTable[int_num], (uint64_t)schedGet(redirectTableMeta[int_num])->pageTable); // give control to the driver
        lapicEOI();                                                                                                     // todo: send eoi only if the task asks us in the syscall
        return;                                                                                                         // don't execute rest of the handler
    }

    if (stack->cs == 0x23) // userspace
    {
        struct sock_socket *initSocket = sockGet(1);

        const char *name = schedGetCurrent(smpID())->name;

        logWarn("%s has crashed with %s at %x! Terminating it.", name, exceptions[int_num], stack->rip);

        if (initSocket)
        {
            char *str = pmmPage();

            // construct a string based on the format "crash %s", name
            memcpy(str, "crash ", 6);
            memcpy(str + 6, name, strlen(name));

            sockAppend(initSocket, str, strlen(str)); // announce that the application has crashed

            pmmDeallocate(str);
        }

        schedKill(schedGetCurrent(smpID())->id); // terminate the task

        sti();        // enable interrupts
        return hlt(); // force a reschedule
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