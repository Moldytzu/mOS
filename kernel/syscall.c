#include <syscall.h>
#include <idt.h>
#include <vmm.h>
#include <scheduler.h>
#include <sys/sys.h>

extern void sysretInit();
extern void SyscallIntHandlerEntry();

// handler called on syscall
void syscallHandler(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SYSCALL_DEBUG
    printks("syscall: number 0x%x, argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNumber, rsi, rdx, returnAddress, r8, r9);
#endif

    if (syscallNumber < (sizeof(syscallHandlers) / sizeof(void *)))                                               // check if the syscall is in range
        syscallHandlers[syscallNumber](syscallNumber, rsi, rdx, returnAddress, r8, 0, r9, schedulerGetCurrent()); // call the handler

    vmmSwap(schedulerGetCurrent()->pageTable); // swap the page table back
}

// init syscall handling
void syscallInit(uint16_t vector)
{
    printk("Installing system call handler on vector 0x%x...", vector);
    idtSetGate((void *)SyscallIntHandlerEntry, vector, IDT_InterruptGateU, true);
    printk("done\n");

    sysretInit(); // enable sysret/syscall capability
}