#include <cpu/lapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <sched/hpet.h>
#include <misc/logger.h>
#include <mm/vmm.h>

void lapicHandleTimer(idt_intrerrupt_stack_t *stack)
{
    logInfo("lapic tick!");

    lapicEOI();
}

void lapicWrite(uint64_t offset, uint32_t value)
{
    *((uint32_t *)((uint8_t *)lapicBase() + offset)) = value;
}

uint32_t lapicRead(uint64_t offset)
{
    return *((uint32_t *)((uint8_t *)lapicBase() + offset));
}

void *lapicBase()
{
    return (void *)(rdmsr(MSR_APIC_BASE) & 0xFFFFFFFFFFFFF000); // clear first 11 bits
}

void lapicEOI()
{
    lapicWrite(APIC_REG_EOI, 0);
}

void lapicInit()
{
    // disable pic by masking all interrupts
    outb(PIC_MASTER_DAT, 0b11111111);
    outb(PIC_SLAVE_DAT, 0b11111111);

    // map the base
    vmmMap(vmmGetBaseTable(), lapicBase(), lapicBase(), false, true, true);

    // enable the lapic
    uint32_t low = (uint64_t)lapicBase() >> 32;
    uint32_t high = (uint64_t)lapicBase() | 0b100000000000; // set global enable flag

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    lapicWrite(APIC_REG_SIV, lapicRead(APIC_REG_SIV) | 0b10000000); // set apic software enable/disable

    lapicWrite(APIC_REG_TPR, 0); // don't block any interrupt

    // set up timer
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011);         // divide by 1
    lapicWrite(APIC_REG_TIMER_INITCNT, 0xFFFFFFFF); // enable timer
    hpetSleepMillis(1);                             // the longer this is, the slower the timer will be

    uint32_t tickRate = 0xFFFFFFFF - lapicRead(APIC_REG_TIMER_CURRENTCNT);

    lapicWrite(APIC_REG_LVT_TIMER, 32 | 0b100000000000000000); // periodic mode
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011);                    // divide by 1
    lapicWrite(APIC_REG_TIMER_INITCNT, tickRate);              // go!

    // enable interrupts
    lapicWrite(APIC_REG_DFR, 0xFFFFFFFF);
    lapicWrite(APIC_REG_LDR, 0x01000000);

    lapicWrite(APIC_REG_SVR, 0x1FF);
}