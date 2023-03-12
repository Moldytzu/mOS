#include <cpu/lapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <cpu/smp.h>
#include <sched/hpet.h>
#include <sched/smpsched.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <main/panic.h>

extern void lapicEntry();

uint64_t lapicTPS[K_MAX_CORES]; // will need this to increase / decrease quantum of each task (we want the same cpu time even if the timer is faster or slower)
uint64_t __tps[K_MAX_CORES];
uint64_t lastSeconds[K_MAX_CORES];

void lapicHandleTimer(idt_intrerrupt_stack_t *stack)
{
    if (hpetMillis() / 1000 != lastSeconds[smpID()])
    {
        // logInfo("lapic tick! %d hz", lapicTPS[smpID()]);

        lapicTPS[smpID()] = __tps[smpID()];

        __tps[smpID()] = 0;
    }

    __tps[smpID()]++;
    lastSeconds[smpID()] = hpetMillis() / 1000;

    schedSchedule(stack);

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
    return (void *)(rdmsr(MSR_APIC_BASE) & 0xFFFFFFFFFFFFF000);
}

void lapicEOI()
{
    lapicWrite(APIC_REG_EOI, 0);
}

void lapicInit(bool bsp)
{
    sti();

    if (bsp)
    {
        // disable pic by masking all interrupts
        outb(PIC_MASTER_DAT, 0b11111111);
        outb(PIC_SLAVE_DAT, 0b11111111);

        // map the base
        vmmMap(vmmGetBaseTable(), lapicBase(), lapicBase(), false, true, false, true); // disable cache
    }

    // reset important registers to a known state before enabling the apic (not required by any spec)
    lapicWrite(APIC_REG_DFR, 0xFF000000);
    lapicWrite(APIC_REG_LDR, 0x01000000);
    lapicWrite(APIC_REG_SVR, 0x100 | 0xFF); // software enable apic and set the spurious vector to 0xFF
    lapicWrite(APIC_REG_TPR, 0);

    // enable the lapic
    uint32_t low = (uint64_t)rdmsr(MSR_APIC_BASE) >> 32;
    uint32_t high = (uint64_t)rdmsr(MSR_APIC_BASE) | 0b100000000000; // set global enable flag

    if (bsp) // set the bsp flag
        high |= (uint32_t)0b100000000;

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    // set up timer
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011); // divide by 1

    lapicWrite(APIC_REG_TIMER_INITCNT, 0xFFFFFFFF); // enable timer
    hpetSleepMillis(1);
    lapicWrite(APIC_REG_TIMER_INITCNT, 0); // disable timer

    uint32_t tickRate = 0xFFFFFFFF - lapicRead(APIC_REG_TIMER_CURRENTCNT);

    lapicWrite(APIC_REG_LVT_TIMER, 0b100000000000000000 | APIC_TIMER_VECTOR); // periodic mode
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011);                                   // divide by 1
    lapicWrite(APIC_REG_TIMER_INITCNT, tickRate);                             // go!

    // setup idt entry if bsp
    if (bsp)
        idtSetGate(lapicEntry, 0x20, IDT_InterruptGateU, true);

    logInfo("lapic: initialised ID: %x", lapicRead(APIC_REG_ID));
}
