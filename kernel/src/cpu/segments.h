#pragma once
#include <misc/utils.h>

ifunc void segmentLoadGS(uint16_t value)
{
    iasm("mov %0, %%gs" ::"r"(value));
}

ifunc uint16_t segmentReadGS()
{
    uint16_t value;
    iasm("mov %%gs, %0" ::"r"(value));
    return value;
}