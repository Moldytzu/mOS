#include <sys/sys.h>
#include <fs/vfs.h>
#include <subsys/vt.h>
#include <drv/serial.h>

// write (rsi = buffer, rdx = count, r8 = fd)
uint64_t write(uint64_t buffer, uint64_t count, uint64_t fd, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(buffer) && count > 1) // prevent a crash
        return SYSCALL_STATUS_ERROR;

    const char *charBuffer = (const char *)PHYSICAL(buffer); // get physical address of the buffer

    if (task->isDriver) // log driver messsages on serial
        for (size_t i = 0; i < count; i++)
            serialWritec(charBuffer[i]);

    if (fd == SYS_STDOUT)
    {
        struct vt_terminal *t = vtGet(task->terminal); // terminal of the task
        vtAppend(t, charBuffer, count);                // append to the terminal
    }
    else
    {
        vfsWrite(FD_TO_NODE(fd), (void *)charBuffer, count, 0); // write to the descriptor

        // todo: also support writing to another terminal
    }

    return SYSCALL_STATUS_OK;
}