#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    uint64_t used, available;
    sys_mem(SYS_MEM_INFO, (uint64_t)&used, (uint64_t)&available);

    printf("Used: %d KiB; Available: %d KiB\n", used / 1024, available / 1024);
}