#include <fw/acpi.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <cpu/idt.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <drv/pcie.h>

uint8_t revision;
acpi_rsdp_t *rsdp;
acpi_sdt_t *sdt;
acpi_mcfg_t *mcfg;

// get a descriptor table with a signature
acpi_sdt_t *acpiGet(const char *sig, int index)
{
    bool xsdt = sdt->signature[0] == 'X';              // XSDT's signature is XSDT, RSDT's signature is RSDT
    size_t entries = sdt->length - sizeof(acpi_sdt_t); // initial value is the length in bytes of the entire tables

    // determine entries count
    if (xsdt)
        entries /= sizeof(uint64_t);
    else
        entries /= sizeof(uint32_t);

    for (size_t i = 0; i < entries; i++)
    {
        acpi_sdt_t *t;

        // xsdt uses 64 bit pointers while rsdt uses 32 bit pointers
        if (xsdt)
            t = (acpi_sdt_t *)(((uint64_t *)(((acpi_xsdt_t *)sdt)->entries))[i]);
        else
            t = (acpi_sdt_t *)(((uint32_t *)(((acpi_xsdt_t *)sdt)->entries))[i]);

        if (memcmp8((void *)sig, t->signature, 4) == 0 && index-- == 0) // compare the signatures
            return t;
    }

    return NULL; // return nothing
}

// checks if pcie ecam is supported by machine
bool pciECAM()
{
    return mcfg;
}

// reboot without acpi
void rebootFallback()
{
    while (inb(0x64) & 0b10) // spin until input buffer of the 8042 is empty
        pause();

    outb(0x64, 0xFE); // send 0xFE (command to pulse line 0xE that is connected to cpu's reset) to the 8042
}

// shutdown without acpi
void shutdownFallback()
{
    // these work only in hypervisors and emulators
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
}

// reboot using acpi
void acpiReboot()
{
    // page 81 of ACPI spec 6.5 (August 29 2022)
    acpi_fadt_t *fadt = (acpi_fadt_t *)acpiGet("FACP", 0);
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

    logError("acpi: reboot unsupported. trying fallback.");
    rebootFallback();
    hang();
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

// initialize the acpi subsystem
void acpiInit()
{
    // get rsdp
    rsdp = (acpi_rsdp_t *)bootloaderGetRSDP();

    vmmMapKernel(rsdp, (void *)((uint64_t)rsdp - (uint64_t)bootloaderGetHHDM()), VMM_ENTRY_RW); // properly map the rsdp

    // parse the version field
    revision = rsdp->version;

    // set the system descriptor table root based on the revision
    if (revision == 0)
        sdt = (void *)(uint64_t)rsdp->rsdt;
    else if (revision >= 2)
        sdt = (void *)rsdp->xsdt;

#ifdef K_ACPI_DEBUG
    logDbg(LOG_SERIAL_ONLY, "acpi: revision %d", revision);
#endif

#ifdef K_PCIE
    // get mcfg
    mcfg = (acpi_mcfg_t *)acpiGet("MCFG", 0);

    if (mcfg)
        pcieEnumerateECAM(mcfg);
    else
        logWarn("acpi: MCFG table wasn't found, PCIe support will not be available");
#endif
}