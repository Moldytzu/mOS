#include <sys.h>
#include <stdio.h>

#define COM1 0x3F8

void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

void _mdrvmain()
{
    printf("started test driver!\n");

    while(1);
}