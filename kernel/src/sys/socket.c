#include <sys/sys.h>
#include <fs/vfs.h>
#include <subsys/socket.h>

// (rsi = call, rbx = arg1, r8 = arg2, r9 = arg3)
void socket(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, sched_task_t *task)
{
    uint64_t *arg1Ptr = PHYSICAL(arg1);
    uint64_t *arg2Ptr = PHYSICAL(arg2);
    uint64_t *arg3Ptr = PHYSICAL(arg2);
    struct sock_socket *s;

    switch (call)
    {
    case 0: // socket create
        if (!arg1Ptr || !INBOUNDARIES(arg1))
            return;

        *arg1Ptr = sockCreate()->id; // create a new socket then return it's id
        break;
    case 1: // socket write
        s = sockGet(arg1);
        if (!s || !INBOUNDARIES(arg2))
            return;

        sockAppend(s, (const char *)arg2Ptr, arg3);
        break;
    case 2: // socket read
        s = sockGet(arg1);
        if (!s || !INBOUNDARIES(arg2))
            return;

        sockRead(s, (const char *)arg2Ptr, arg3);
        break;
    case 3:                // socket destroy
        s = sockGet(arg1); // get the socket
        if (!s)
            return;

        sockDestroy(s);
        break;
    default:
        break;
    }
}