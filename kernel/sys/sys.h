#pragma once
#include <utils.h>

#define SYS_STDIN 1

#define PHYSICAL(virtual) ((void *)(vmmGetPhys(task->pageTable, (void *)(virtual))))

#define STACK alignD(task->intrerruptStack.rsp, 4096)
#define INSTACK(address) between(address, STACK - 4096, STACK + 4096)
#define INAPPLICATION(address) between(address, TASK_BASE_ADDRESS, task->lastVirtualAddress + 4096)
#define INBOUNDARIES(address) (INSTACK(((uint64_t)address)) || INAPPLICATION(((uint64_t)address)))

#include <sys/exit.h>
#include <sys/write.h>
#include <sys/read.h>
#include <sys/input.h>
#include <sys/display.h>
#include <sys/exec.h>
#include <sys/pid.h>
#include <sys/mem.h>
#include <sys/vfs.h>
#include <sys/open.h>
#include <sys/close.h>

void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, struct sched_task *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close};