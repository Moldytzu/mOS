#include <fw/acpi.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <main/panic.h>
#include <misc/logger.h>

// reboot without acpi
void rebootFallback()
{
    while (inb(0x64) & 0b10) // spin until input buffer of the 8042 is empty
        pause();

    outb(0x64, 0xFE); // send 0xFE (command to pulse line 0xE that is connected to cpu's reset) to the 8042
}

// reboot using acpi
void acpiReboot()
{
    // page 81 of ACPI spec 6.5 (August 29 2022)
    if (fadt)
    {
        switch (fadt->reset.addressSpace)
        {
        case ACPI_GAS_ACCESS_MEMORY:
            *((uint8_t *)fadt->reset.address) = fadt->resetValue;
            break;
        case ACPI_GAS_ACCESS_IO:
            outb((uint16_t)fadt->reset.address & 0xFFFF, fadt->resetValue);
            break;
        default:
            panick("acpi: unsupported fadt reset address space (expected System Memory or System I/O)");
            break;
        }
    }

    logError("acpi: reboot unsupported. trying fallback.");
    rebootFallback();
    hang();
}

// shutdown without acpi
void shutdownFallback()
{
    // these work only in hypervisors and emulators
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
}

// shutdown using acpi
void acpiShutdown()
{
    logError("acpi: shutdown unsupported. trying emulator-only fallback.");

    shutdownFallback();

    logError("acpi: fallback failed.");
    panick("It is safe to shutdown computer!");
    hang();
}