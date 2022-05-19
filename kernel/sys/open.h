#include <sys/sys.h>
#include <vfs.h>

// (rsi = returnPtr, rbx = path)
void open(uint64_t syscallNumber, uint64_t returnPtr, uint64_t path, uint64_t returnAddress, uint64_t r8, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(returnPtr) || !INBOUNDARIES(path)) // be sure that the arguments are under the stack
        return;

    uint64_t *returnVal = PHYSICAL(returnPtr);
    uint64_t fd = vfsOpen(PHYSICAL(path)); // open the node
    *returnVal = fd;                       // return the value
}