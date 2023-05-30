#pragma once
#include <stdint.h>
#include <stddef.h>

#define SYS_INPUT_KEYBOARD 0

#define SYS_DISPLAY_MODE 0
#define SYS_DISPLAY_SET 1
#define SYS_DISPLAY_GET 2

#define SYS_DISPLAY_TTY 1
#define SYS_DISPLAY_FB 2

#define SYS_PID_STATUS 0
#define SYS_PID_GET_ENVIROMENT 1
#define SYS_PID_SET_ENVIROMENT 2
#define SYS_PID_GET 3
#define SYS_PID_GET_CWD 4
#define SYS_PID_SET_CWD 5
#define SYS_PID_SLEEP 6

#define SYS_MEM_ALLOCATE 0

#define SYS_VFS_FILE_EXISTS 0
#define SYS_VFS_DIRECTORY_EXISTS 1
#define SYS_VFS_LIST_DIRECTORY 2
#define SYS_VFS_FILE_SIZE 3

#define SYS_SOCKET_CREATE 0
#define SYS_SOCKET_WRITE 1
#define SYS_SOCKET_READ 2
#define SYS_SOCKET_DESTROY 3

#define SYS_POWER_REBOOT 0
#define SYS_POWER_SHUTDOWN 1
#define SYS_POWER_UPTIME 2

#define SYS_TIME_GET_UPTIME_NANOS 0

#define SYS_PERF_GET_MEMORY 0
#define SYS_PERF_GET_CPU 1

#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_INPUT 3
#define SYS_DISPLAY 4
#define SYS_EXEC 5
#define SYS_PID 6
#define SYS_MEM 7
#define SYS_VFS 8
#define SYS_OPEN 9
#define SYS_CLOSE 10
#define SYS_SOCKET 11
#define SYS_POWER 12
#define SYS_DRIVER 13
#define SYS_TIME 14
#define SYS_PERF 15

typedef struct __attribute__((packed))
{
    uint8_t shouldCreateNewTerminal;
    const char *enviroment;
    const char *cwd;
    int argc;
    char **argv;
} sys_exec_packet_t;

extern void _syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);
void sys_exit(uint64_t status);
void sys_write(void *buffer, uint64_t count, uint64_t fd);
void sys_read(void *buffer, uint64_t count, uint64_t fd);
void sys_input(uint8_t deviceType, char *returnPtr);
void sys_display(uint8_t call, uint64_t arg1, uint64_t arg2);
void sys_exec(const char *path, uint64_t *pid, sys_exec_packet_t *packet);

void sys_pid(uint32_t pid, uint16_t info, uint64_t *retVal);
uint64_t sys_pid_get();

void sys_time(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_time_uptime_nanos();

void sys_mem(uint8_t call, uint64_t arg1, uint64_t arg2);
void sys_vfs(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_open(const char *path);
void sys_close(uint64_t fd);
void sys_socket(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void sys_power(uint8_t call, uint64_t arg1, uint64_t arg2);
void sys_driver(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3);
void sys_perf(uint8_t call, uint64_t arg1, uint64_t arg2);
void sys_yield();