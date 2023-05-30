#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    uint64_t unused;
    uint64_t used, available, cores;
    sys_perf(SYS_PERF_GET_MEMORY, (uint64_t)&used, (uint64_t)&available);
    sys_perf(SYS_PERF_GET_CPU, (uint64_t)&cores, (uint64_t)&unused);

    printf("Used: %d KiB; Available: %d KiB\n", used / 1024, available / 1024);
    printf("Cores: %d\n", cores);
}