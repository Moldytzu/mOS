#pragma once

#define SYS_STDIN 1

#define PHYSICAL(virtual) ((void *)(vmmGetPhys(task->pageTable,(void *)(virtual))))

#include <utils.h>
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