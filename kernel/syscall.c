#include <syscall.h>
#include <idt.h>

extern void SyscallHandlerEntry();

void syscallHandler(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9)
{
    printk("Syscall no' %d ",rdi);
}

void syscallInit(uint16_t vector)
{
    printk("Installing system call handler on vector 0x%x...",vector);
    idtSetGate((void*)SyscallHandlerEntry,vector,IDT_InterruptGateU);
    printk("done\n");
}