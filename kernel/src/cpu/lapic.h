#pragma once
#include <misc/utils.h>

#define MSR_APIC_BASE 0x1B
#define APIC_REG_SPURIOUS_INT_VECTOR 0xF0
#define APIC_REG_EOI 0xB0

void lapicInit();
void *lapicBase();
void lapicWrite(uint64_t offset, uint32_t value);
void lapicEOI();