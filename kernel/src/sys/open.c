#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = path)
uint64_t open(uint64_t path, uint64_t rbx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(path)) // be sure that the arguments are under the stack
        return SYSCALL_STATUS_ERROR;

    uint64_t node = sysOpenRelativePath(PHYSICAL(path), task); // open the file

    if (!node)
        return 0;

    // find first empty file descriptor
    for (int i = 0; i < TASK_MAX_FILE_DESCRIPTORS; i++)
    {
        if (task->fileDescriptorPointers[i])
            continue;

        task->fileDescriptorPointers[i] = node;
        logDbg(LOG_SERIAL_ONLY, "vfs: opening %s as fd %d", PHYSICAL(path), i + 2);
        return i + 2;
    }

    vfsClose(node);
    return 0;
}