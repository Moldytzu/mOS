#include <syscall.h>
#include <idt.h>
#include <vmm.h>
#include <scheduler.h>

void *taskTable; // page table of the current task

extern void sysretInit();
extern void SyscallIntHandlerEntry();

// exit (rsi = exit status)
void exit(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
#ifdef K_SYSCALL_DEBUG
    printks("syscall: exit status %d\n\r", rsi);
#endif
    schedulerGetCurrent()->state = 1; // stop
}

// write (rsi = buffer, rdx = count, r8 = fd
void write(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    // todo: check the fd and then print, if it's 1 then write to the terminal's buffer
    const char *buffer = vmmGetPhys(taskTable, (void *)rsi); // get physical address of the buffer
    for (size_t i = 0; i < rdx; i++)
        printk("%c", buffer[i]);
}

// lookup table of syscall handlers
void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t) = {exit, write};

// handler called on syscall
void syscallHandler(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory
    taskTable = schedulerGetCurrent()->pageTable;

#ifdef K_SYSCALL_DEBUG
    printks("syscall: number 0x%x, argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNumber, rsi, rdx, returnAddress, r8, r9);
#endif

    if (syscallNumber < (sizeof(syscallHandlers) / sizeof(void *))) // check if the syscall is in range
        syscallHandlers[syscallNumber](syscallNumber, rsi, rdx, returnAddress, r8, r9); // call the handler

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