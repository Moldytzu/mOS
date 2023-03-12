#include <sys/syscall.h>
#include <sys/sys.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

extern void sysretInit();

// handler called on syscall
void syscallHandler(idt_intrerrupt_stack_t *registers)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SYSCALL_DEBUG
    printks("syscall: requested %s (0x%x), argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNames[registers->rdi], registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
#endif

    sched_task_t *t = schedGetCurrent(smpID());

    if (registers->rdi < (sizeof(syscallHandlers) / sizeof(void *))) // check if the syscall is in range
    {
        logDbg(1, "%s calls %s", t->name, syscallNames[registers->rdi]);
        syscallHandlers[registers->rdi](registers->rsi, registers->rdx, registers->r8, registers->r9, t); // call the handler
    }

    vmmSwap((void *)registers->cr3); // swap the page table back
}

// init syscall handling
void syscallInit()
{
    sysretInit(); // enable sysret/syscall capability
    logInfo("syscall: enabled syscall/sysret functionality");
}