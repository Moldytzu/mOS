#include <fw/acpi.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <main/panic.h>
#include <misc/logger.h>

void amlInit()
{
    fadt = (acpi_fadt_t *)acpiGet("FACP", 0);                         // grab the fadt
    dsdt = (acpi_dsdt_t *)(fadt->DSDT64 ? fadt->DSDT64 : fadt->DSDT); // use the correct dsdt address

    logInfo("acpi: found dsdt at %p", dsdt);
}