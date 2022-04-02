#include <syscall.h>
#include <idt.h>
#include <vmm.h>
#include <scheduler.h>

extern void SyscallHandlerEntry();

void syscallHandler(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9)
{
    void *taskTable = schedulerGetCurrent()->pageTable;
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

    printk("Syscall no' %d ", rdi);
    vmmSwap(taskTable); // swap the page table back
}

void syscallInit(uint16_t vector)
{
    printk("Installing system call handler on vector 0x%x...", vector);
    idtSetGate((void *)SyscallHandlerEntry, vector, IDT_InterruptGateU);
    printk("done\n");
}