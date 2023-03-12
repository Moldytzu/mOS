#pragma once
#include <misc/utils.h>
#include <sched/scheduler.h>

sched_task_t *elfLoad(const char *path, int argc, char **argv, bool driver);