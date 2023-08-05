#include <sys/sys.h>
#include <fs/vfs.h>
#include <subsys/socket.h>

// (rsi = call, rbx = arg1, r8 = arg2, r9 = arg3)
uint64_t socket(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, sched_task_t *task)
{
    uint64_t *arg1Ptr = PHYSICAL(arg1);
    uint64_t *arg2Ptr = PHYSICAL(arg2);
    uint64_t *arg3Ptr = PHYSICAL(arg2);
    struct sock_socket *s;

    switch (call)
    {
    case 0:                      // socket create
        return sockCreate()->id; // create a new socket then return it's id

    case 1: // socket write
        s = sockGet(arg1);
        if (!s || !arg2)
            return SYSCALL_STATUS_ERROR;

        sockAppend(s, (const char *)arg2Ptr, arg3);
        return SYSCALL_STATUS_OK;

    case 2: // socket read
        s = sockGet(arg1);
        if (!s || !arg2)
            return SYSCALL_STATUS_ERROR;

        sockRead(s, (const char *)arg2Ptr, arg3);
        return SYSCALL_STATUS_OK;

    case 3:                // socket destroy
        s = sockGet(arg1); // get the socket
        if (!s)
            return SYSCALL_STATUS_ERROR;

        sockDestroy(s);
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}