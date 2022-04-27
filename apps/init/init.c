#include <sys.h>

void sys_exit(uint64_t status)
{
    _syscall(SYS_EXIT,status,0,0,0,0); // sys_exit
    while(1); // endless loop 
}

int main()
{
    return 1024;
}