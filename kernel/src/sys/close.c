#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = fd)
void close(uint64_t fd, uint64_t rbx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    // todo: add safe-guards for apps to not give arbritary file descriptors (maybe give apps numerical ids?????)
    vfsClose(fd); // close the node
}