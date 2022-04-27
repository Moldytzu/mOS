#include <stdint.h>
#include <stddef.h>

extern void _syscall(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9);

void _mmain()
{
    _syscall(1024,0,0,0,0,0);
    while(1); // endless loop
}