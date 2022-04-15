#include <utils.h>

// load new cr4
static inline __attribute__((always_inline)) void controlLoadCR4(uint64_t value)
{
    iasm("mov %0, %%cr4" ::"r"(value));
}

// load new cr3
static inline __attribute__((always_inline)) void controlLoadCR3(uint64_t value)
{
    iasm("mov %0, %%cr3" ::"r"(value));
}