#include <cpu/xapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <cpu/smp.h>
#include <sched/time.h>
#include <sched/hpet.h>
#include <sched/scheduler.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <main/panic.h>

extern void xapicEntry();

// #define TPS

#ifdef TPS
uint64_t lapicTPS[K_MAX_CORES];
uint64_t __tps[K_MAX_CORES];
uint64_t lastSeconds[K_MAX_CORES];
#endif
bool isEnabled = false;

void xapicHandleTimer(idt_intrerrupt_stack_t *stack)
{
    vmmSwap(vmmGetBaseTable()); // swap to base table

#ifdef TPS
    // the scheduler will be available here and we will deadlock
    // thus we have to disable rescheduling
#ifdef K_ATOMIC_RESCHEDULE
#error atomic rescheduling is not available if printing lapic timer tps
#endif

    if (hpetMillis() / 1000 != lastSeconds[smpID()])
    {
        lapicTPS[smpID()] = __tps[smpID()];

        logInfo("%d", lapicTPS[smpID()]);

        __tps[smpID()] = 0;
    }

    __tps[smpID()]++;
    lastSeconds[smpID()] = hpetMillis() / 1000;
#endif

    xapicEOI();           // send end of interrupt
    schedSchedule(stack); // reschedule
}

ifunc void xapicWrite(uint64_t offset, uint32_t value)
{
    *((volatile uint32_t *)((uint8_t *)XAPIC_BASE + offset)) = value;
}

ifunc uint32_t xapicRead(uint64_t offset)
{
    return *((volatile uint32_t *)((uint8_t *)XAPIC_BASE + offset));
}

void xapicEOI()
{
    xapicWrite(XAPIC_REG_EOI, 0);
}

uint64_t xapicCalibrateHPET(uint64_t divisor)
{
    // 11.5.4 volume 3 intel sdm

    // calibrate timer for the frequency specified in config
    xapicWrite(XAPIC_REG_TIMER_DIV, divisor);
    xapicWrite(XAPIC_REG_LVT_TIMER, XAPIC_TIMER_VECTOR); // oneshot mode
    xapicWrite(XAPIC_REG_TIMER_INITCNT, 0xFFFFFFFF);     // initialise with -1

    // determine ticks per period
    hpetSleepNanos((1000000000 /*one sec in nanos*/ / K_LAPIC_FREQ));
    uint32_t ticks = 0xFFFFFFFF - xapicRead(XAPIC_REG_TIMER_CURRENTCNT);

    xapicWrite(XAPIC_REG_LVT_TIMER, XAPIC_TIMER_VECTOR | (1 << 17)); // periodic mode
    xapicWrite(XAPIC_REG_TIMER_INITCNT, ticks);                      // initialise with calibrated tick
}

void xapicInit(bool bsp)
{
    if (rdmsr(MSR_APIC_BASE) & 0xFFFFF000 != (uint64_t)XAPIC_BASE)
        panick("Out of spec xapic address.");

    if (bsp)
    {
        // disable pic by masking all interrupts
        outb(PIC_MASTER_DAT, 0b11111111);
        outb(PIC_SLAVE_DAT, 0b11111111);

        // map the base
        vmmMapKernel(XAPIC_BASE, XAPIC_BASE, VMM_ENTRY_RW | VMM_ENTRY_CACHE_DISABLE);

        // set the isr to one that doesn't screw up the stack on an interrupt that doesn't have an error code like the timer's one while calling the same general interrupt entry point (check cpu/isr.asm)
        idtSetGate(xapicEntry, XAPIC_TIMER_VECTOR);

        isEnabled = true;
    }

    // reset important registers to a known state before enabling the apic (not required by any spec)
    xapicWrite(XAPIC_REG_DFR, 0xFF000000);
    xapicWrite(XAPIC_REG_LDR, 0x01000000);
    xapicWrite(XAPIC_REG_LVT0, 0x00010000);
    xapicWrite(XAPIC_REG_LVT1, 0x00010000);
    xapicWrite(XAPIC_REG_TPR, 0);

    // enable the lapic (11.4.3 volume 3 intel sdm)
    uint32_t low = (uint64_t)rdmsr(MSR_APIC_BASE) >> 32;
    uint32_t high = (uint64_t)rdmsr(MSR_APIC_BASE) | 0b100000000000; // set global enable flag

    if (bsp) // set the bsp flag
        high |= (uint32_t)0b100000000;

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    xapicWrite(XAPIC_REG_SIV, 0x120); // software enable apic and set the spurious vector to 0x20

    // perform calibration
    logInfo("xapic: calibrated with 0x%x initcnt", xapicCalibrateHPET(0 /*divisor of 2*/));
}

// sends a non-maskable interrupt to all of the cores thus halting them
void xapicNMI()
{
#ifdef K_SMP
    if (!isEnabled)
    {
        logError("xapic: trying to NMI with disabled xapic");
        return;
    }

    // 11.6 volume 3 intel sdm

    uint64_t icr = 0;
    icr |= (0b100) << 8; // set delivery mode to nmi
    icr |= (0b11) << 18; // set destination shorthand to all excluding self

    // send the interrupt command register
    xapicWrite(XAPIC_REG_ICR_LOW, icr & 0xFFFFFFFF);
    xapicWrite(XAPIC_REG_ICR_HIGH, (icr >> 32) & 0xFFFFFFFF);
#endif
}