#pragma once
#include <misc/utils.h>

// load new cr4
ifunc void controlLoadCR4(uint64_t value)
{
    iasm("mov %0, %%cr4" ::"r"(value));
}

// load new cr3
ifunc void controlLoadCR3(uint64_t value)
{
    iasm("mov %0, %%cr3" ::"r"(value));
}

// read cr2
ifunc uint64_t controlReadCR2()
{
    uint64_t value;
    iasm("mov %%cr2, %0" ::"r"(value));
    return value;
}

// read cr3
ifunc uint64_t controlReadCR3()
{
    uint64_t value;
    iasm("mov %%cr3, %0" ::"r"(value));
    return value;
}

// read cr4
ifunc uint64_t controlReadCR4()
{
    uint64_t value;
    iasm("mov %%cr4, %0" ::"r"(value));
    return value;
}