#include <cpu/lapic.h>
#include <cpu/msr.h>
#include <cpu/pic.h>
#include <cpu/io.h>
#include <misc/logger.h>
#include <mm/vmm.h>

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
    uint32_t high = (uint64_t)lapicBase() | 0b100100000000; // set global enable flag and processor is bsp flag

    wrmsr(MSR_APIC_BASE, low, high); // write back the base

    lapicWrite(APIC_REG_SPURIOUS_INT_VECTOR, lapicRead(APIC_REG_SPURIOUS_INT_VECTOR) | 0b10000000); // set apic software enable/disable

    logInfo("lapic: present at %x", lapicBase());
}