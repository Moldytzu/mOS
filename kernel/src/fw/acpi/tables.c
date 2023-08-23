#include <fw/acpi.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <drv/pcie.h>
#include <sched/hpet.h>

acpi_rsdp_t *rsdp;
acpi_xsdt_t *sdt;
acpi_mcfg_t *mcfg;
acpi_fadt_t *fadt;
acpi_dsdt_t *dsdt;

// get a descriptor table with a signature
acpi_sdt_t *acpiGet(const char *sig, int index)
{
    bool xsdt = sdt->header.signature[0] == 'X';              // XSDT's signature is XSDT, RSDT's signature is RSDT
    size_t entries = sdt->header.length - sizeof(acpi_sdt_t); // initial value is the length in bytes of the entire tables

    // determine entries count
    if (xsdt)
        entries /= sizeof(uint64_t);
    else
        entries /= sizeof(uint32_t);

    for (size_t i = 0; i < entries; i++)
    {
        acpi_sdt_t *entry;

        // xsdt uses 64 bit pointers while rsdt uses 32 bit pointers
        if (xsdt)
            entry = (acpi_sdt_t *)((uint64_t *)(sdt->entries))[i];
        else
            entry = (acpi_sdt_t *)((uint32_t *)(sdt->entries))[i];

        if (memcmp8((void *)sig, entry->signature, 4) == 0 && index-- == 0) // compare the signatures
            return entry;
    }

    return NULL; // return nothing
}

// initialize the acpi subsystem
void acpiInit()
{
    // get rsdp
    rsdp = (acpi_rsdp_t *)bootloaderGetRSDP();

    vmmMapKernel(rsdp, (void *)((uint64_t)rsdp - (uint64_t)bootloaderGetHHDM()), VMM_ENTRY_RW); // properly map the rsdp (fixme: we should probably check the rsdp length)

    // parse the version field
    uint8_t version = rsdp->version;

    // set the system descriptor table root based on the version
    if (version == 0)
        sdt = (void *)(uint64_t)rsdp->rsdt;
    else if (version >= 2)
        sdt = (void *)rsdp->xsdt;

#ifdef K_ACPI_DEBUG
    logDbg(LOG_SERIAL_ONLY, "acpi: version %d", version);
#endif

#ifdef K_ACPI_AML
    amlInit(); // initialise the aml parser
#endif

    hpetInit(); // initialise the hpet

#ifdef K_PCIE
    // get mcfg
    mcfg = (acpi_mcfg_t *)acpiGet("MCFG", 0);

    if (mcfg)
        pcieEnumerateECAM(mcfg);
    else
        logWarn("acpi: MCFG table wasn't found, PCIe support will not be available");
#endif
}