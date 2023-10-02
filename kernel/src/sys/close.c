#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = fd)
uint64_t close(uint64_t fd, uint64_t rbx, uint64_t r8, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    if (!FD_TO_NODE(fd))
        return SYSCALL_STATUS_ERROR;

    vfsClose(FD_TO_NODE(fd));
    task->fileDescriptorPointers[fd - 2] = 0; // in open we offset by 2 to skip ids 0 and 1 which have other purposese

    return SYSCALL_STATUS_OK;
}