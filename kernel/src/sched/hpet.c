#include <sched/hpet.h>
#include <fw/acpi.h>
#include <mm/vmm.h>
#include <main/panic.h>

#define HPET_OFFSET_GENERAL_CAPABILITIES 0
#define HPET_OFFSET_GENERAL_CONFIGURATION 0x10
#define HPET_OFFSET_GENERAL_INTERRUPT 0x20
#define HPET_OFFSET_MAIN_COUNTER 0xF0
#define HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(n) (0x100 + 0x20 * n)
#define HPET_OFFSET_TIMER_COMPARATOR(n) (0x108 + 0x20 * n)
#define HPET_OFFSET_TIMER_INTERRUPT(n) (0x110 + 0x20 * n)

acpi_hpet_t *hpet;
uint64_t hpetNano;

void hpetWrite(uint64_t offset, uint64_t data)
{
    switch (hpet->base.addressSpace)
    {
    case ACPI_GAS_ACCESS_MEMORY:
        *((uint64_t *)(hpet->base.address + offset)) = data;
        break;

        // todo: add more address space modes if necessary

    default:
        panick("Failed HPET write! Unsupported address space.");
        break;
    }
}

uint64_t hpetRead(uint64_t offset)
{
    switch (hpet->base.addressSpace)
    {
    case ACPI_GAS_ACCESS_MEMORY:
        return *((uint64_t *)(hpet->base.address + offset));
        break;

        // todo: add more address space modes if necessary

    default:
        panick("Failed HPET read! Unsupported address space.");
        break;
    }

    return 0;
}

void hpetSleepNanos(uint64_t nanos)
{
    uint64_t targetTime = hpetNanos() + nanos;

    while (hpetNanos() < targetTime)
        pause();
}

void hpetSleepMillis(uint64_t millis)
{
    uint64_t targetTime = hpetMillis() + millis;

    while (hpetMillis() < targetTime)
        pause();
}

uint64_t hpetNanos()
{
    return hpetRead(HPET_OFFSET_MAIN_COUNTER) * hpetNano;
}

uint64_t hpetMillis()
{
    return hpetNanos() / 1000000;
}

bool hpetAvailable()
{
    return hpet;
}

void hpetInit()
{
    hpet = (acpi_hpet_t *)acpiGet("HPET");

    if (!hpet) // didn't find it
        return;

    if (hpet->base.addressSpace == ACPI_GAS_ACCESS_MEMORY)
        vmmMap(vmmGetBaseTable(), (void *)hpet->base.address, (void *)hpet->base.address, false, true); // map if it is in memory

    uint64_t hpetFemto = (hpetRead(HPET_OFFSET_GENERAL_CAPABILITIES) >> 32) & 0xFFFFFFFF;
    hpetNano = hpetFemto / 1000000; // convert units of measure (femto to nano)

    hpetWrite(HPET_OFFSET_MAIN_COUNTER, 0);                                        // clear the counter
    hpetWrite(HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(0), (1 << 3) | (1 << 6)); // configure the timer (periodic timer-specific configuration)
    hpetWrite(HPET_OFFSET_TIMER_COMPARATOR(0), 0);                                 // clear the comparator
    hpetWrite(HPET_OFFSET_GENERAL_CONFIGURATION, 0b1);                             // enable counter
}