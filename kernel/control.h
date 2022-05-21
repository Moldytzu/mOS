#pragma once
#include <utils.h>

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