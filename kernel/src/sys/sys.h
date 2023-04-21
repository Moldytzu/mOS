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

char *expandPath(const char *path, sched_task_t *task);

void exit(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void write(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void read(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void input(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void display(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void exec(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void pid(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void mem(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void vfs(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void open(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void close(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void socket(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void power(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);
void driver(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *);