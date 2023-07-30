#include <sys/sys.h>
#include <fs/vfs.h>

// read (rsi = buffer, rdx = count, r8 = fd)
void read(uint64_t buffer, uint64_t count, uint64_t fd, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(buffer)) // prevent a crash
        return;

    void *charBuffer = PHYSICAL(buffer); // get physical address of the buffer

    vfsRead(FD_TO_NODE(fd), charBuffer, count, 0); // read from the vfs

    // todo: also support reading from another terminal
}