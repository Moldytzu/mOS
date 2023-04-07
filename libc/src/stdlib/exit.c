#include <stdlib.h>
#include <mos/sys.h>

void exit(int status)
{
    while(1) // todo: fix schedKill then remove this
        asm volatile("int $0x20"); 
    sys_exit(status);
}
