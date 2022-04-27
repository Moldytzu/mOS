#include <stdint.h>
#include <stddef.h>

#define SYS_EXIT 0

extern void _syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);