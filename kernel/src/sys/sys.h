#pragma once
#include <misc/utils.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <fs/vfs.h>
#include <sched/scheduler.h>

#define SYS_STDIN 1

#define PHYSICAL(virtual) ((void *)(vmmGetPhys((void *)task->pageTable, (void *)(virtual))))

#define STACK alignD(task->registers.rsp, 4096)
#define INSTACK(address) between(address, STACK - K_STACK_SIZE, STACK + K_STACK_SIZE)
#define INAPPLICATION(address) between(address, TASK_BASE_ADDRESS, task->lastVirtualAddress + 4096)
#define INBOUNDARIES(address) (INSTACK(((uint64_t)address)) || INAPPLICATION(((uint64_t)address)))

// expand relative path to full path
ifunc char *expandPath(const char *path, sched_task_t *task)
{
    uint64_t fd;
    char *buffer = (char *)pmmPage();

    if (memcmp(path, "./", 2) == 0) // check if the task requests from the current directory
    {
        path += 2; // skip ./
        goto cwd;
    }

    memset64(buffer, 0, VMM_PAGE / sizeof(uint64_t)); // clear the buffer
    memcpy(buffer, path, strlen(path));               // copy the input

    // check if it exists
    fd = vfsOpen(buffer);
    if (fd)
    {
        vfsClose(fd);
        return buffer;
    }

cwd: // copy the cwd before the input
    uint64_t offset = strlen(task->cwd);
    memset64(buffer, 0, VMM_PAGE / sizeof(uint64_t));
    memcpy((void *)(buffer + offset), path, strlen(path));
    memcpy((void *)buffer, task->cwd, offset);

    // check if it exists
    fd = vfsOpen(buffer);
    if (fd)
    {
        vfsClose(fd);
        return buffer;
    }

    // fail
    pmmDeallocate(buffer);
    return NULL;
}

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
#include <sys/socket.h>
#include <sys/power.h>
#include <sys/driver.h>

void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close, socket, power, driver};
const char *syscallNames[] = {"exit", "write", "read", "input", "display", "exec", "pid", "mem", "vfs", "open", "close", "socket", "power", "driver"};