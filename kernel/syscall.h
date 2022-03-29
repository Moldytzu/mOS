#pragma once
#include <utils.h>

uint64_t syscallHandler(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);
void syscallInit(uint16_t vector);