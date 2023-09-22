#include <fw/acpi.h>
#include <misc/logger.h>
#include <cpu/io.h>

bool present;
bool extendedTimer;

uint32_t acpiTimerRead()
{
    if (!present)
        return 0;

    switch (fadt->PMTimer64.addressSpace)
    {
    case ACPI_GAS_ACCESS_MEMORY:
        return *(uint32_t *)fadt->PMTimer64.address;
    case ACPI_GAS_ACCESS_IO:
        return ind(fadt->PMTimer64.address);
    default:
        break;
    }
}

void acpiTimerInit()
{
    if (!fadt) // no fadt/aml loaded
        return;

    extendedTimer = fadt->flags & ACPI_FADT_TMR_VAL_EXT; // extended timer has 32 bits instead of 24
    present = fadt->PMTimer64.address > 0;

    if (!present) // unsupported
        return;

    logInfo("acpi: %s power management timer is at %p of %d access", extendedTimer ? "extended" : "standard", fadt->PMTimer64.address, fadt->PMTimer64.addressSpace);
}