#pragma once
#include <misc/utils.h>
#include <sched/scheduler.h>

struct sched_task *elfLoad(const char *path, int argc, char **argv, bool driver);