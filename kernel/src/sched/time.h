#pragma once
#include <misc/utils.h>

#define TIME_SOURCE_NONE 0
#define TIME_SOURCE_HPET 1

void timeSource();
uint64_t timeNanos();