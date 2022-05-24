#include <sys/sys.h>
#include <vfs.h>

// (rsi = fd)
void close(uint64_t fd, uint64_t rbx, uint64_t r8, uint64_t r9, struct sched_task *task)
{
    if(fd == 0)
        return;

    vfsClose(fd); // close the node
}