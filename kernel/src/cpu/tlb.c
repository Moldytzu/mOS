#include <cpu/tlb.h>
#include <mm/vmm.h>
#include <cpu/control.h>

// flush tlb
volatile void tlbFlushAll()
{
    vmmSwap((void *)controlReadCR3());
}

void tlbFlush(void *ptr)
{
    asm volatile("invlpg (%0)" ::"r" (ptr) : "memory");
}