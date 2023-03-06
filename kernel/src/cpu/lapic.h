#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

#define MSR_APIC_BASE 0x1B

#define APIC_REG_ID 0x20
#define APIC_REG_SIV 0xF0
#define APIC_REG_EOI 0xB0
#define APIC_REG_TPR 0x80
#define APIC_REG_DFR 0xE0
#define APIC_REG_LDR 0xD0
#define APIC_REG_SVR 0xF0
#define APIC_REG_TIMER_DIV 0x3E0
#define APIC_REG_TIMER_INITCNT 0x380
#define APIC_REG_TIMER_CURRENTCNT 0x390
#define APIC_REG_LVT_TIMER 0x320

#define APIC_TIMER_VECTOR 0x20

void lapicInit(bool bsp);
void *lapicBase();
void lapicWrite(uint64_t offset, uint32_t value);
void lapicEOI();
void lapicHandleTimer(idt_intrerrupt_stack_t *stack);
void lapicEnableTimer();