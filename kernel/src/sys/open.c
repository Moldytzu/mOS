#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = returnPtr, rbx = path)
void open(uint64_t returnPtr, uint64_t path, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(returnPtr) || !IS_MAPPED(path)) // be sure that the arguments are under the stack
        return;

    uint64_t *returnVal = PHYSICAL(returnPtr);
    *returnVal = openRelativePath(PHYSICAL(path), task); // open the file
}