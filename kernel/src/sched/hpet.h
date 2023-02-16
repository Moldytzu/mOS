#pragma once
#include <misc/utils.h>

uint64_t hpetNanos();
uint64_t hpetMillis();

void hpetSleepNanos(uint64_t nanos);
void hpetSleepMillis(uint64_t millis);

void hpetInit();
bool hpetAvailable();