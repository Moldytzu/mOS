#pragma once
#include <utils.h>

static inline void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}