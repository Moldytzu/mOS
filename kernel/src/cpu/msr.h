#pragma once
#include <cpu/io.h>
#include <misc/utils.h>

uint64_t rdmsr(uint32_t ecx);
void wrmsr(uint32_t ecx, uint64_t val);