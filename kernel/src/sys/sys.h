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
#define SYSCALL_STATUS_OK 0
#define SYSCALL_STATUS_ERROR 1
#define SYSCALL_STATUS_UNKNOWN_OPERATION 2
#define SYSCALL_STATUS_ACCESS_DENIED 3

#define PHYSICAL(virtual) ((void *)(vmmGetPhysical((void *)task->pageTable, (void *)(virtual))))
#define IS_MAPPED(address) ((uint64_t)PHYSICAL(address) > 0)
#define DEFINE_SYSCALL(x) uint64_t x(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
#define FD_TO_NODE(x) (((x)-2) < TASK_MAX_FILE_DESCRIPTORS ? task->fileDescriptorPointers[(x)-2] : 0) // in open we offset by 2 to skip ids 0 and 1 which have other purposese
#define IS_PRIVILEGED ((task->isDriver) || (task->id == 1))                                           // privileged apps are drivers and init system

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