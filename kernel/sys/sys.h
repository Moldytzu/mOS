#pragma once
#include <utils.h>
#include <pmm.h>
#include <vmm.h>
#include <vfs.h>

#define SYS_STDIN 1

#define PHYSICAL(virtual) ((void *)(vmmGetPhys(task->pageTable, (void *)(virtual))))

#define STACK alignD(task->intrerruptStack.rsp, 4096)
#define INSTACK(address) between(address, STACK - 4096, STACK + 4096)
#define INAPPLICATION(address) between(address, TASK_BASE_ADDRESS, task->lastVirtualAddress + 4096)
#define INBOUNDARIES(address) (INSTACK(((uint64_t)address)) || INAPPLICATION(((uint64_t)address)))

ifunc char *expandPath(const char *path, struct sched_task *task)
{
    uint64_t fd;
    char *buffer = mmAllocatePage();

    if (memcmp(path, "./", 2) == 0) // check if the task requests from the current directory
    {
        path += 2; // skip ./
        goto cwd;
    }

    memset64(buffer, 0, VMM_PAGE / sizeof(uint64_t)); // clear the buffer
    memcpy(buffer, path, strlen(path));     // copy the input

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
    mmDeallocatePage(buffer);
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

void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, struct sched_task *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close};