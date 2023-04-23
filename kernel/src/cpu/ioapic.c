#include <cpu/ioapic.h>
#include <fw/acpi.h>
#include <misc/logger.h>

acpi_madt_t *madt;

pstruct
{
    uint8_t type;
    uint8_t len;
}
madt_entry_t;

pstruct
{
    madt_entry_t header;
    uint8_t bus;
    uint8_t irq;
    uint32_t systemInt;
    uint16_t flags;
}
ioapic_iso_t;

ioapic_iso_t *interruptOverrides[256];
uint16_t interruptOverridesIdx = 0;

void ioapicInit()
{
    zero(interruptOverrides, sizeof(interruptOverrides));
    
    madt = (acpi_madt_t *)acpiGet("APIC", 0); // retrive the madt table

    madt_entry_t *e = (madt_entry_t *)((uint64_t)madt + sizeof(acpi_madt_t)); // point to the first entry
    for (uint64_t s = 0; s < madt->header.length - sizeof(acpi_madt_t);)      // iterate over all entries
    {
        if (e->type == 2) // IO/APIC Interrupt Source Override
            interruptOverrides[interruptOverridesIdx++] = (ioapic_iso_t *)e;

        s += e->len;
        e = (madt_entry_t *)((uint64_t)e + e->len); // point to next entry
    }

    for (uint64_t i = 0; i < interruptOverridesIdx; i++)
        logInfo("irq %d from bus %d -> %d", interruptOverrides[i]->irq, interruptOverrides[i]->bus, interruptOverrides[i]->systemInt);
}