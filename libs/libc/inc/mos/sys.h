#pragma once
#include <stdint.h>
#include <stddef.h>

#define SYS_INPUT_KEYBOARD 0
#define SYS_INPUT_MOUSE 1

#define SYS_DISPLAY_MODE 0
#define SYS_DISPLAY_SET 1
#define SYS_DISPLAY_GET 2
#define SYS_DISPLAY_MAP_FB 3
#define SYS_DISPLAY_UPDATE_FB 4

#define SYS_DISPLAY_TTY 1
#define SYS_DISPLAY_FB 2
#define SYS_DISPLAY_FB_DOUBLE_BUFFERED 3

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

#define SYS_MAILBOX_READ_FIRST 0

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
#define SYS_MAILBOX 16

typedef struct __attribute__((packed))
{
    uint8_t shouldCreateNewTerminal;
    const char *enviroment;
    const char *cwd;
    int argc;
    char **argv;
} sys_exec_packet_t;

typedef struct
{
    uint8_t reserved[9];

    uint32_t sender; // sender's pid
    size_t subject;  // numeric subject

    size_t messageLength; // length of message
    char message[];       // message
} mailbox_t;

extern uint64_t _syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);
void sys_exit(uint64_t status);
uint64_t sys_write(void *buffer, uint64_t count, uint64_t fd);
uint64_t sys_read(void *buffer, uint64_t count, uint64_t fd);
char sys_input_keyboard();
void sys_input_mouse(uint16_t *x, uint16_t *y);
uint64_t sys_display(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_exec(const char *path, uint64_t *pid, sys_exec_packet_t *packet);

uint64_t sys_pid(uint64_t pid, uint64_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_pid_get();

uint64_t sys_time(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_time_uptime_nanos();

uint64_t sys_mem(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_vfs(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_open(const char *path);
uint64_t sys_close(uint64_t fd);
uint64_t sys_socket(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3);
uint64_t sys_power(uint8_t call, uint64_t arg1, uint64_t arg2);
uint64_t sys_driver(uint8_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3);
uint64_t sys_perf(uint8_t call, uint64_t arg1, uint64_t arg2);

mailbox_t *sys_mailbox_read();

void sys_yield();