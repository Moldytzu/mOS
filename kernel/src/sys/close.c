#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = fd)
void close(uint64_t fd, uint64_t rbx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    fd -= 2; // in open we offset by 2 to skip ids 0 and 1 which have other purposese

    if (!FD_TO_NODE(fd))
        return;

    vfsClose(FD_TO_NODE(fd));
    task->fileDescriptorPointers[fd] = 0;
}