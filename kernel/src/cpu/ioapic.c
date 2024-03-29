#include <cpu/ioapic.h>
#include <cpu/pic.h>
#include <fw/acpi.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <main/panic.h>

#define IRQ_TO_OFFSET(x) (0x10 + 2 * x)

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

pstruct
{
    madt_entry_t header;
    uint8_t id;
    uint8_t pad;
    uint32_t address;
    uint32_t systemIntBase;
}
ioapic_t;

void *ioapicBase;
ioapic_t *ioapic;
ioapic_iso_t *interruptOverrides[256];
uint16_t interruptOverridesIdx = 0;

void ioapicWrite(uint8_t offset, uint32_t value)
{
    *(volatile uint32_t *)ioapicBase = offset;                   // register selector
    *(volatile uint32_t *)((uint64_t)ioapicBase + 0x10) = value; // write actual data
}

uint32_t ioapicRead(uint8_t offset)
{
    *(volatile uint32_t *)ioapicBase = offset;                  // register selector
    return *(volatile uint32_t *)((uint64_t)ioapicBase + 0x10); // read data
}

void ioapicRedirectIRQ(uint8_t irq, uint16_t vector, uint16_t core)
{
    for (int i = 0; i < interruptOverridesIdx; i++)
    {
        if (interruptOverrides[i]->irq == irq) // check for redirection in madt
            irq = interruptOverrides[i]->systemInt;
    }

    uint64_t redirector = vector;       // redirect to vector 0x21
    redirector |= (uint64_t)core << 56; // set destination core

    ioapicWrite(IRQ_TO_OFFSET(irq), redirector & 0xFFFFFFFF);             // write low bits
    ioapicWrite(IRQ_TO_OFFSET(irq) + 1, (redirector >> 32) & 0xFFFFFFFF); // write high bits

    logInfo("ioapic: redirected irq 0x%x to vector 0x%x of core %d", irq, vector, core);
}

void ioapicInit()
{
    zero(interruptOverrides, sizeof(interruptOverrides));

    madt = (acpi_madt_t *)acpiGet("APIC", 0); // retrive the madt table

    madt_entry_t *e = (madt_entry_t *)((uint64_t)madt + sizeof(acpi_madt_t)); // point to the first entry
    for (uint64_t s = 0; s < madt->header.length - sizeof(acpi_madt_t);)      // iterate over all entries
    {
        switch (e->type)
        {
        case 1: // I/O APIC
            ioapic = (ioapic_t *)e;
            ioapicBase = (void *)((uint64_t)ioapic->address);
            vmmMapKernel(ioapicBase, ioapicBase, VMM_ENTRY_RW);
            break;
        case 2: // IO/APIC Interrupt Source Override
            interruptOverrides[interruptOverridesIdx++] = (ioapic_iso_t *)e;
            break;
        }

        s += e->len;
        e = (madt_entry_t *)((uint64_t)e + e->len); // point to next entry
    }

    if (!ioapicBase)
        panick("MADT doesn't contain I/O APIC base");
}