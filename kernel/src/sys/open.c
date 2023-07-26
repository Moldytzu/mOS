#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = returnPtr, rbx = path)
void open(uint64_t returnPtr, uint64_t path, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(returnPtr) || !IS_MAPPED(path)) // be sure that the arguments are under the stack
        return;

    uint64_t *returnVal = PHYSICAL(returnPtr);
    uint64_t node = openRelativePath(PHYSICAL(path), task); // open the file

    if (!node)
    {
        *returnVal = 0;
        return;
    }

    // find first empty file descriptor
    for (int i = 0; i < TASK_MAX_FILE_DESCRIPTORS; i++)
    {
        if (task->fileDescriptorPointers[i])
            continue;

        task->fileDescriptorPointers[i] = node;
        *returnVal = i + 2; // 0 is reserved for fail and 1 is reserved for STDOUT

        logDbg(LOG_SERIAL_ONLY, "vfs: opening %s as fd %d", PHYSICAL(path), *returnVal);
        return;
    }

    // fail
    *returnVal = 0;

    vfsClose(node);
}