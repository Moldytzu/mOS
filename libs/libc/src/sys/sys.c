#include <mos/sys.h>

void sys_exit(uint64_t status)
{
    _syscall(SYS_EXIT, status, 0, 0, 0, 0);
}

uint64_t sys_write(void *buffer, uint64_t count, uint64_t fd)
{
    return _syscall(SYS_WRITE, (uint64_t)buffer, count, fd, 0, 0);
}

uint64_t sys_read(void *buffer, uint64_t count, uint64_t fd)
{
    return _syscall(SYS_READ, (uint64_t)buffer, count, fd, 0, 0);
}

char sys_input_keyboard()
{
    return _syscall(SYS_INPUT, SYS_INPUT_KEYBOARD, 0, 0, 0, 0);
}

void sys_input_mouse(uint16_t *x, uint16_t *y)
{
    _syscall(SYS_INPUT, SYS_INPUT_MOUSE, (uint64_t)x, (uint64_t)y, 0, 0);
}

uint64_t sys_display(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_DISPLAY, call, arg1, arg2, 0, 0);
}

uint64_t sys_exec(const char *path, uint64_t *pid, sys_exec_packet_t *packet)
{
    return _syscall(SYS_EXEC, (uint64_t)path, (uint64_t)pid, (uint64_t)packet, 0, 0);
}

uint64_t sys_pid(uint64_t pid, uint64_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_PID, pid, call, arg1, arg2, 0);
}

uint64_t sys_mem(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_MEM, call, arg1, arg2, 0, 0);
}

uint64_t sys_vfs(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_VFS, call, arg1, arg2, 0, 0);
}

uint64_t sys_open(const char *path)
{
    return _syscall(SYS_OPEN, (uint64_t)path, 0, 0, 0, 0);
}

uint64_t sys_close(uint64_t fd)
{
    return _syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
}

uint64_t sys_socket(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    return _syscall(SYS_SOCKET, call, arg1, arg2, arg3, 0);
}

uint64_t sys_power(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_POWER, call, arg1, arg2, 0, 0);
}

uint64_t sys_time(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_TIME, call, arg1, arg2, 0, 0);
}

uint64_t sys_time_uptime_nanos()
{
    return sys_time(SYS_TIME_GET_UPTIME_NANOS, 0, 0);
}

uint64_t sys_driver(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    return _syscall(SYS_DRIVER, call, arg1, arg2, arg3, 0);
}

void sys_yield()
{
    asm volatile("int $0x20"); // simulate timer intrerrupt
}

uint64_t sys_pid_get()
{
    return sys_pid(0, SYS_PID_GET, 0, 0);
}

uint64_t sys_perf(uint8_t call, uint64_t arg1, uint64_t arg2)
{
    return _syscall(SYS_PERF, call, arg1, arg2, 0, 0);
}

uint64_t sys_mailbox(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4)
{
    return _syscall(SYS_MAILBOX, call, arg1, arg2, arg3, arg4);
}

mail_t *sys_mailbox_read()
{
    return (mail_t *)_syscall(SYS_MAILBOX, SYS_MAILBOX_READ_FIRST, 0, 0, 0, 0);
}

uint64_t sys_mailbox_compose(uint64_t pid, char *buffer, size_t bufferLength, uint64_t subject)
{
    return sys_mailbox(SYS_MAILBOX_COMPOSE, pid, (uint64_t)buffer, bufferLength, subject);
}