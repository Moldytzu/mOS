#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

static inline void cpuid(uint32_t reg, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "0"(reg));
}

int main(int argc, char **argv)
{
    uint32_t largestStandardFunc;

    char vendor[13], name[49];
    vendor[12] = name[48] = 0;

    cpuid(0, &largestStandardFunc, (uint32_t *)(vendor + 0), (uint32_t *)(vendor + 8), (uint32_t *)(vendor + 4));       // read vendor
    cpuid(0x80000002, (uint32_t *)(name + 0), (uint32_t *)(name + 4), (uint32_t *)(name + 8), (uint32_t *)(name + 12)); // read brand
    cpuid(0x80000003, (uint32_t *)(name + 16), (uint32_t *)(name + 20), (uint32_t *)(name + 24), (uint32_t *)(name + 28));
    cpuid(0x80000004, (uint32_t *)(name + 32), (uint32_t *)(name + 36), (uint32_t *)(name + 40), (uint32_t *)(name + 44));

    uint64_t unused;
    uint64_t used, available, cores;
    sys_perf(SYS_PERF_GET_MEMORY, (uint64_t)&used, (uint64_t)&available);
    sys_perf(SYS_PERF_GET_CPU, (uint64_t)&cores, (uint64_t)&unused);

    printf("Used: %d KiB; Available: %d KiB\n", used / 1024, available / 1024);
    printf("CPU %d * %s (%s)\n", cores, name, vendor);
}