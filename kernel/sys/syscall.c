#include <sys/syscall.h>
#include <sys/sys.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>

extern void sysretInit();
extern void SyscallIntHandlerEntry();
uint32_t count = 1;

// handler called on syscall
void optimize syscallHandler(struct idt_intrerrupt_stack *registers)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SYSCALL_DEBUG
    printks("syscall: requested %s (0x%x), argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNames[registers->rdi], registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
#endif

    struct sched_task *t = schedulerGetCurrent();

    t->syscallUsage++; // increase the syscall usage
    count++;

    //sti();
    if (registers->rdi < (sizeof(syscallHandlers) / sizeof(void *)))                                      // check if the syscall is in range
        syscallHandlers[registers->rdi](registers->rsi, registers->rdx, registers->r8, registers->r9, t); // call the handler
    //cli();

    vmmSwap((void *)registers->cr3); // swap the page table back
}

// get the count of used syscalls
uint32_t syscallGetCount()
{
    register uint32_t tmp = count; // store the count
    count = 1;                     // reset the count
    return tmp;                    // return the count
}

// init syscall handling
void syscallInit(uint16_t vector)
{
    printk("Installing system call handler on vector 0x%x and enabling sysret/syscall...", vector);
    idtSetGate((void *)SyscallIntHandlerEntry, vector, IDT_InterruptGateU, true); // set up the gate
    sysretInit();                                                                 // enable sysret/syscall capability
    printk("done\n");
}