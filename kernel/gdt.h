#pragma once
#include <utils.h>

struct pack GDTDescriptor
{
    uint16_t size;
    uint64_t offset;
};

struct pack GDTSegment
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    unsigned limit2 : 4;
    unsigned flags : 4;
    uint8_t base3;
};

void GDTCreateSegment(uint8_t access, uint8_t offset);
void GDTInit();