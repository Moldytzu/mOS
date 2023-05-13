#include <sys/sys.h>
#include <sched/time.h>

void time(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // get system uptime in nanoseconds
        if(!INBOUNDARIES(arg1))
            return;
        
        uint64_t *arg1Ptr = PHYSICAL(arg1);
        *arg1Ptr = timeNanos();
        break;
    
    default:
        break;
    }
}