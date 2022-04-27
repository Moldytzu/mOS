#include <syscall.h>
#include <idt.h>
#include <vmm.h>
#include <scheduler.h>

extern void sysretInit();
extern void SyscallIntHandlerEntry();

// handler called on syscall
void syscallHandler(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    void *taskTable = schedulerGetCurrent()->pageTable;
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SYSCALL_DEBUG
    printks("syscall: number %d, argument 1 is %d, argument 2 is %d, return address is %p, argument 3 is %d, argument 4 is %d\n\r", syscallNumber, rsi, rdx, returnAddress, r8, r9);
#endif

    switch (syscallNumber)
    {
    case 0: // exit
#ifdef K_SYSCALL_DEBUG
        printks("syscall: exit status %d\n\r", rsi);
#endif
        schedulerGetCurrent()->state = 1; // stop
        break;

    default:
        break;
    };

    vmmSwap(taskTable); // swap the page table back
}

// init syscall handling
void syscallInit(uint16_t vector)
{
    printk("Installing system call handler on vector 0x%x...", vector);
    idtSetGate((void *)SyscallIntHandlerEntry, vector, IDT_InterruptGateU, true);
    printk("done\n");

    sysretInit(); // enable sysret/syscall capability
}