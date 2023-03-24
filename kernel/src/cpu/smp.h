#pragma once
#include <misc/utils.h>
#include <cpu/tlb.h>

uint8_t smpID();
uint8_t smpCores();
void smpBootstrap();
void smpJumpUserspace();