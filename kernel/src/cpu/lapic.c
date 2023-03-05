#include <cpu/lapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <misc/logger.h>
#include <mm/vmm.h>

void *lapicBase()
{
    return (void *)(rdmsr(MSR_APIC_BASE) & 0xFFFFFFFFFFFFF000); // clear first 11 bits
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
    uint32_t high = (uint64_t)lapicBase() | 0b100100000000; // set global enable flag and processor is bsp flag

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    logInfo("lapic: present at %x", lapicBase());
}