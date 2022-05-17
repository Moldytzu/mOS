#include <sys.h>

void sys_exit(uint64_t status)
{
    _syscall(SYS_EXIT, status, 0, 0, 0, 0);
    while (1)
        ; // endless loop
}

void sys_write(void *buffer, uint64_t count, uint64_t fd)
{
    _syscall(SYS_WRITE, (uint64_t)buffer, count, fd, 0, 0);
}

void sys_read(void *buffer, uint64_t count, uint64_t fd)
{
    _syscall(SYS_READ, (uint64_t)buffer, count, fd, 0, 0);
}

void sys_input(uint8_t deviceType, char *returnPtr)
{
    _syscall(SYS_INPUT, deviceType, (uint64_t)returnPtr, 0, 0, 0);
}

void sys_display(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    _syscall(SYS_DISPLAY, call, arg1, arg2, 0, 0);
}

void sys_exec(const char *path, uint64_t *pid, struct sys_exec_packet *packet)
{
    _syscall(SYS_EXEC, (uint64_t)path, (uint64_t)pid, (uint64_t)packet, 0, 0);
}

void sys_pid(uint32_t pid, uint16_t info, uint64_t *retVal)
{
    _syscall(SYS_PID, pid, info, (uint64_t)retVal, 0, 0);
}

void sys_mem(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    _syscall(SYS_MEM, call, arg1, arg2, 0, 0);
}

void sys_vfs(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    _syscall(SYS_VFS, call, arg1, arg2, 0, 0);
}

void sys_open(const char *path, uint64_t *fd)
{
    _syscall(SYS_OPEN, (uint64_t)path, (uint64_t)fd, 0, 0, 0);
}

void sys_close(uint64_t fd)
{
    _syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}