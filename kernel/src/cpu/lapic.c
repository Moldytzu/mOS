#include <cpu/lapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <cpu/smp.h>
#include <sched/hpet.h>
#include <sched/scheduler.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <main/panic.h>

extern void lapicEntry();

uint64_t lapicTPS[K_MAX_CORES];
uint64_t __tps[K_MAX_CORES];
uint64_t lastSeconds[K_MAX_CORES];
bool isEnabled = false;

void lapicHandleTimer(idt_intrerrupt_stack_t *stack)
{
    if (hpetMillis() / 1000 != lastSeconds[smpID()])
    {
        lapicTPS[smpID()] = __tps[smpID()];

        // logInfo("%d", lapicTPS[smpID()]);

        __tps[smpID()] = 0;
    }

    __tps[smpID()]++;
    lastSeconds[smpID()] = hpetMillis() / 1000;

    schedSchedule(stack);

    lapicEOI();
}

uint64_t *lapicGetTPS()
{
    return lapicTPS;
}

void lapicWrite(uint64_t offset, uint32_t value)
{
    *((uint32_t *)((uint8_t *)APIC_BASE + offset)) = value;
}

uint32_t lapicRead(uint64_t offset)
{
    return *((uint32_t *)((uint8_t *)APIC_BASE + offset));
}

void lapicEOI()
{
    lapicWrite(APIC_REG_EOI, 0);
}

void lapicInit(bool bsp)
{
    if (bsp)
    {
        // disable pic by masking all interrupts
        outb(PIC_MASTER_DAT, 0b11111111);
        outb(PIC_SLAVE_DAT, 0b11111111);

        // map the base
        vmmMap(vmmGetBaseTable(), APIC_BASE, APIC_BASE, VMM_ENTRY_RW | VMM_ENTRY_CACHE_DISABLE);

        isEnabled = true;
    }

    // reset important registers to a known state before enabling the apic (not required by any spec)
    lapicWrite(APIC_REG_DFR, 0xFF000000);
    lapicWrite(APIC_REG_LDR, 0x01000000);
    lapicWrite(APIC_REG_SIV, 0x1FF); // software enable apic and set the spurious vector to 0xFF
    lapicWrite(APIC_REG_TPR, 0);

    // enable the lapic
    uint32_t low = (uint64_t)rdmsr(MSR_APIC_BASE) >> 32;
    uint32_t high = (uint64_t)rdmsr(MSR_APIC_BASE) | 0b100000000000; // set global enable flag

    if (bsp) // set the bsp flag
        high |= (uint32_t)0b100000000;

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    // set up timer to a frequency ~1 kHz (todo: real hardware crashes here, not sure why)
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011);            // divide by 1
    lapicWrite(APIC_REG_TIMER_INITCNT, 10000000);       // enable timer
    lapicWrite(APIC_REG_LVT_TIMER, APIC_TIMER_VECTOR); // one shot mode

    uint64_t before = hpetMillis();

    while (lapicRead(APIC_REG_TIMER_CURRENTCNT))
        ; // wait for the timer to clear

    uint64_t after = hpetMillis();

    uint64_t target = 10000000 / (after - before + 1 /*that 1 prevents us dividing by 0 and generating an exception*/);

    lapicWrite(APIC_REG_LVT_TIMER, 0b100000000000000000 | APIC_TIMER_VECTOR); // periodic mode
    lapicWrite(APIC_REG_TIMER_DIV, 0b1011);                                   // divide by 1
    lapicWrite(APIC_REG_TIMER_INITCNT, target);                               // go!

    // setup idt entry (only needed to do once)
    if (bsp)
        idtSetGate(lapicEntry, APIC_TIMER_VECTOR, IDT_InterruptGateU, true); // set the isr to one that doesn't screw up the stack on an interrupt that doesn't have an error code like the timer's one while calling the same general interrupt entry point (check cpu/isr.asm)

    logInfo("lapic: initialised ID: %x", lapicRead(APIC_REG_ID));
}

// sends a non-maskable interrupt to all of the cores thus halting them
void lapicNMI()
{
    if (!isEnabled)
        return;

    // 11.6 volume 3 intel sdm

    uint64_t icr = 0;
    icr |= (0b100) << 8; // set delivery mode to nmi
    icr |= (0b11) << 18; // set destination shorthand to all excluding self

    // send the interrupt command register
    lapicWrite(APIC_REG_ICR_LOW, icr & 0xFFFFFFFF);
    lapicWrite(APIC_REG_ICR_HIGH, (icr >> 32) & 0xFFFFFFFF);
}