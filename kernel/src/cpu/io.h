#pragma once
#include <misc/utils.h>

ifunc void outb(uint16_t port, uint8_t val) // out byte
{
    iasm("outb %0, %1" ::"a"(val), "Nd"(port));
    iasm("outb %0, %1" ::"a"((uint8_t)0), "Nd"((uint16_t)0x80)); // wait a bit
}

ifunc uint8_t inb(uint16_t port) // in byte
{
    uint8_t val;
    iasm("inb %%dx,%%al"
         : "=a"(val)
         : "d"(port));
    return val;
}

ifunc void outw(uint16_t port, uint16_t val) // out byte
{
    iasm("outw %0, %1" ::"a"(val), "Nd"(port));
}

ifunc uint16_t inw(uint16_t port) // in byte
{
    uint16_t val;
    iasm("inw %%dx,%%ax"
         : "=a"(val)
         : "d"(port));
    return val;
}