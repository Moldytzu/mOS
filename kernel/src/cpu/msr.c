#include <cpu/msr.h>

uint64_t rdmsr(uint32_t ecx)
{
    uint32_t low, high;
    iasm("rdmsr"
         : "=a"(low), "=d"(high)
         : "c"(ecx));

    return ((uint64_t)high << 32) | low;
}

void wrmsr(uint32_t ecx, uint32_t low, uint32_t high)
{
    iasm("wrmsr" ::"a"(high), "d"(low), "c"(ecx));
}