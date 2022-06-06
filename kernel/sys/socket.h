#include <sys/sys.h>
#include <vfs.h>
#include <socket.h>

// (rsi = call, rbx = arg1, r8 = arg2)
void socket(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    uint64_t *arg1Ptr = PHYSICAL(arg1);
    uint64_t *arg2Ptr = PHYSICAL(arg2);
    switch (call)
    {
    case 0: // socket create
        if(!arg1Ptr)
            return;
    
        *arg1Ptr = sockCreate()->id; // create a new socket then return it's id
        break;
    default:
        break;
    }
}