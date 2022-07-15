#pragma once
#include <misc/utils.h>
#include <cpu/pic.h>
#include <cpu/idt.h>

#define PIT_BASE 0x40
#define PIT_CH_0 (PIT_BASE + 0)
#define PIT_CH_1 (PIT_BASE + 1)
#define PIT_CH_2 (PIT_BASE + 2)
#define PIT_CMD (PIT_BASE + 3)

#define PIT_DIV 1193182

struct pack pit_packet
{
    unsigned binarymode : 1;
    unsigned operatingmode : 3;
    unsigned accessmode : 2;
    unsigned channel : 2;
};

uint64_t pitGetTicks();
uint32_t pitGetScale();
void pitSet(uint32_t hz);
void pitInit();