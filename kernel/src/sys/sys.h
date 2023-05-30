#pragma once
#include <misc/utils.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <fs/vfs.h>
#include <sched/scheduler.h>

// macros for internal system call implementations only

#define SYS_STDIN 1

#define PHYSICAL(virtual) ((void *)(vmmGetPhys((void *)task->pageTable, (void *)(virtual))))
#define STACK alignD(task->registers.rsp, 4096)
#define INSTACK(address) between(address, STACK - K_STACK_SIZE, STACK + K_STACK_SIZE)
#define INAPPLICATION(address) between(address, TASK_BASE_ADDRESS, task->lastVirtualAddress + 4096)
#define INBOUNDARIES(address) (INSTACK(((uint64_t)address)) || INAPPLICATION(((uint64_t)address)))
#define DEFINE_SYSCALL(x) void x(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);

uint64_t openRelativePath(const char *path, sched_task_t *task);
void yield();

DEFINE_SYSCALL(exit);
DEFINE_SYSCALL(write);
DEFINE_SYSCALL(read);
DEFINE_SYSCALL(input);
DEFINE_SYSCALL(display);
DEFINE_SYSCALL(exec);
DEFINE_SYSCALL(pid);
DEFINE_SYSCALL(mem);
DEFINE_SYSCALL(vfs);
DEFINE_SYSCALL(open);
DEFINE_SYSCALL(close);
DEFINE_SYSCALL(socket);
DEFINE_SYSCALL(power);
DEFINE_SYSCALL(driver);
DEFINE_SYSCALL(time);
DEFINE_SYSCALL(perf);