#include <sys.h>

void sys_exit(uint64_t status)
{
    _syscall(SYS_EXIT,status,0,0,0,0); // sys_exit
    while(1); // endless loop 
}

void sys_write(void *buffer, uint64_t count, uint64_t fd)
{
    _syscall(SYS_WRITE,(uint64_t)buffer,count,fd,0,0);
}