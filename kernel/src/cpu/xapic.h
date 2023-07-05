#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

#define MSR_APIC_BASE 0x1B

#define XAPIC_REG_ID 0x20
#define XAPIC_REG_SIV 0xF0
#define XAPIC_REG_EOI 0xB0
#define XAPIC_REG_TPR 0x80
#define XAPIC_REG_DFR 0xE0
#define XAPIC_REG_LDR 0xD0
#define XAPIC_REG_ICR_LOW 0x300
#define XAPIC_REG_ICR_HIGH 0x310
#define XAPIC_REG_TIMER_DIV 0x3E0
#define XAPIC_REG_TIMER_INITCNT 0x380
#define XAPIC_REG_TIMER_CURRENTCNT 0x390
#define XAPIC_REG_LVT_TIMER 0x320

#define XAPIC_TIMER_VECTOR 0x20
#define XAPIC_NMI_VECTOR 0x02

#define XAPIC_BASE ((void *)0xFEE00000)

void xapicNMI();
void xapicInit(bool bsp);
void xapicWrite(uint64_t offset, uint32_t value);
void xapicEOI();
void xapicHandleTimer(idt_intrerrupt_stack_t *stack);
void xapicEnableTimer();