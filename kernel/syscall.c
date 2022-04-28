#include <syscall.h>
#include <idt.h>
#include <vmm.h>
#include <scheduler.h>

extern void sysretInit();
extern void SyscallIntHandlerEntry();

// handler called on syscall
void syscallHandler(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory
    void *taskTable = schedulerGetCurrent()->pageTable;

#ifdef K_SYSCALL_DEBUG
    printks("syscall: number 0x%x, argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNumber, rsi, rdx, returnAddress, r8, r9);
#endif

    // todo: use a lookup table
    switch (syscallNumber)
    {
    case 0: // exit (rsi = exit status)
#ifdef K_SYSCALL_DEBUG
        printks("syscall: exit status %d\n\r", rsi);
#endif
        schedulerGetCurrent()->state = 1; // stop
        break;
    case 1: // write (rsi = buffer, rdx = count, r8 = fd)
        // todo: check the fd and then print, if it's 1 then write to the terminal's buffer
        const char *buffer = vmmGetPhys(taskTable, (void *)rsi); // get physical address of the buffer
        for (size_t i = 0; i < rdx; i++)
            printk("%c", buffer[i]);
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