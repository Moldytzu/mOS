#include <sys/sys.h>

// exit (rsi = exit status)
uint64_t exit(uint64_t status, uint64_t rdx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    logDbg(LOG_SERIAL_ONLY, "pid %d exits with %d", task->id, status);
    schedKill(task->id); // kill the task
    schedSwitchNext();   // switch to next task
    unreachable();       // schedSwitchNext never returns here!

    return SYSCALL_STATUS_OK;
}
