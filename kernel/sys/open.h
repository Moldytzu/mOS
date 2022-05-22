#include <sys/sys.h>
#include <vfs.h>

// (rsi = returnPtr, rbx = path)
void open(uint64_t syscallNumber, uint64_t returnPtr, uint64_t path, uint64_t returnAddress, uint64_t r8, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(returnPtr) || !INBOUNDARIES(path)) // be sure that the arguments are under the stack
        return;

    uint64_t fd;
    char *input = PHYSICAL(path);
    char *buffer = mmAllocatePage();
    uint64_t *returnVal = PHYSICAL(returnPtr);

    if (memcmp(input, "./", 2) == 0) // check if the task requests from the current directory
    {
        input += 2; // skip ./
        goto cwd;
    }

    memset64(buffer, 0, VMM_PAGE / sizeof(uint64_t)); // clear the buffer
    memcpy(buffer, input, strlen(input));             // copy the input

    // check if it exists
    fd = vfsOpen(buffer);
    if (fd)
    {
        *returnVal = fd; // return the value
        goto ret;
    }

cwd: // copy the cwd before the input
    uint64_t offset = strlen(task->cwd);
    memset64(buffer, 0, VMM_PAGE / sizeof(uint64_t));
    memcpy((void *)(buffer + offset), input, strlen(input));
    memcpy((void *)buffer, task->cwd, offset);

    // check if it exists
    fd = vfsOpen(buffer);
    if (fd)
    {
        *returnVal = fd; // return the value
        goto ret;
    }

    *returnVal = 0; // fail

ret:
    mmDeallocatePage(buffer);
}