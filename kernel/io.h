#pragma once
#include <utils.h>

static inline void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
    asm volatile ("outb %0, %1" :: "a"((uint8_t)0), "Nd"((uint16_t)0x80)); // wait a bit
}

static inline uint8_t inb(uint16_t port) // in byte
{
    uint8_t val;
    asm volatile ("inb %%dx,%%al" : "=a" (val) : "d" (port));
    return val;
}