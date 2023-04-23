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

void ioapicInit()
{
    madt = (acpi_madt_t *)acpiGet("APIC", 0); // retrive the madt table

    madt_entry_t *e = (madt_entry_t *)((uint64_t)madt + sizeof(acpi_madt_t)); // point to the first entry
    for (uint64_t s = 0; s < madt->header.length - sizeof(acpi_madt_t);)
    {
        logInfo("type %d %d %d", e->type, e->len, s);
        s += e->len;
        e = (madt_entry_t *)((uint64_t)e + e->len);
    }
}