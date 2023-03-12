#pragma once
#include <sys/sys.h>

// exit (rsi = exit status)
void exit(uint64_t status, uint64_t rdx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
#ifdef K_SYSCALL_DEBUG
    printks("syscall: exit status %d\n\r", status);
#endif
    schedKill(task->id); // kill the task
}
