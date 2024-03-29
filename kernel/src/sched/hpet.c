#include <sched/hpet.h>
#include <sched/time.h>
#include <fw/acpi.h>
#include <mm/vmm.h>
#include <main/panic.h>
#include <misc/logger.h>

#define HPET_OFFSET_GENERAL_CAPABILITIES 0
#define HPET_OFFSET_GENERAL_CONFIGURATION 0x10
#define HPET_OFFSET_GENERAL_INTERRUPT 0x20
#define HPET_OFFSET_MAIN_COUNTER 0xF0
#define HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(n) (0x100 + 0x20 * n)
#define HPET_OFFSET_TIMER_COMPARATOR(n) (0x108 + 0x20 * n)
#define HPET_OFFSET_TIMER_INTERRUPT(n) (0x110 + 0x20 * n)

acpi_hpet_t *hpet;
uint64_t hpetNano;

ifunc void hpetWrite(uint64_t offset, uint64_t data)
{
    *((volatile uint64_t *)(hpet->base.address + offset)) = data;
}

ifunc uint64_t hpetRead(uint64_t offset)
{
    return *((volatile uint64_t *)(hpet->base.address + offset));
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
    hpet = (acpi_hpet_t *)acpiGet("HPET", 0);

    if (!hpet)
    {
        logInfo("hpet: failed to detect");
        return;
    }

    if (hpet->base.addressSpace != ACPI_GAS_ACCESS_MEMORY)
    {
        logInfo("hpet: unsupported address space");
        return;
    }

    vmmMapKernel((void *)hpet->base.address, (void *)hpet->base.address, VMM_ENTRY_RW); // map it

    // calculate the scale
    uint32_t hpetFemto = hpetRead(HPET_OFFSET_GENERAL_CAPABILITIES) >> 32;
    hpetNano = hpetFemto / 1000000; // convert units of measure (femto to nano)

    hpetWrite(HPET_OFFSET_MAIN_COUNTER, 0);                                        // clear the counter
    hpetWrite(HPET_OFFSET_TIMER_CONFIGURATION_CAPABILITY(0), (1 << 3) | (1 << 6)); // configure the timer (periodic timer-specific configuration)
    hpetWrite(HPET_OFFSET_TIMER_COMPARATOR(0), 0);                                 // clear the comparator
    hpetWrite(HPET_OFFSET_GENERAL_CONFIGURATION, 0b1);                             // enable counter

    logInfo("hpet: detected with frequency of %d MHz", 1000000000000000 / hpetFemto / 1000000);

    timeSource();
}