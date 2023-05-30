#include <fw/acpi.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <cpu/idt.h>
#include <main/panic.h>
#include <misc/logger.h>

uint8_t revision;
acpi_rsdp_hdr_t *rsdp;
acpi_sdt_t *sdt;
acpi_mcfg_t *mcfg;

acpi_pci_descriptor_t *pciFuncs = NULL;
uint16_t pciIndex = 0;

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
            t = (acpi_sdt_t *)(((uint64_t *)(((acpi_xsdt_hdr_t *)sdt)->entries))[i]);
        else
            t = (acpi_sdt_t *)(((uint32_t *)(((acpi_xsdt_hdr_t *)sdt)->entries))[i]);

        if (memcmp8((void *)sig, t->signature, 4) == 0 && index-- == 0) // compare the signatures
            return t;
    }

    return NULL; // return nothing
}

// enumerate the pci bus using mcfg and ecam
void acpiEnumeratePCI()
{
    if (!mcfg)
        return;

#ifdef K_PCIE
    size_t entries = (mcfg->header.length - sizeof(acpi_mcfg_t)) / sizeof(acpi_pci_config_t);
    for (int i = 0; i < entries; i++)
    {
        // enumerate each bus
        for (int bus = mcfg->buses[i].startBus; bus < mcfg->buses[i].endBus; bus++)
        {
            uint64_t busBase = mcfg->buses[i].base;
            acpi_pci_header_t *baseHeader = (acpi_pci_header_t *)busBase;
            vmmMap(vmmGetBaseTable(), baseHeader, baseHeader, VMM_ENTRY_RW); // map the header

            // check for non-existent bus
            if (baseHeader->device == UINT16_MAX || baseHeader->device == 0)
                continue;

            // enumerate each device
            for (int device = 0; device < 32; device++)
            {
                acpi_pci_header_t *deviceHeader = (acpi_pci_header_t *)(busBase + (bus << 20 | device << 15));
                vmmMap(vmmGetBaseTable(), deviceHeader, deviceHeader, VMM_ENTRY_RW); // map the header

                // check for non-existent device
                if (deviceHeader->device == UINT16_MAX || deviceHeader->device == 0)
                    continue;

                // enumerate each function
                for (int function = 0; function < 8; function++)
                {
                    acpi_pci_header_t *functionHeader = (acpi_pci_header_t *)(busBase + (bus << 20 | device << 15 | function << 12));

                    // check for non-existent function
                    vmmMap(vmmGetBaseTable(), functionHeader, functionHeader, VMM_ENTRY_RW); // map the header

                    if (functionHeader->device == UINT16_MAX || functionHeader->device == 0)
                        continue;

                    logInfo("acpi: found pci function %x:%x at %d.%d.%d", functionHeader->vendor, functionHeader->device, bus, device, function);

                    // build the descriptor
                    acpi_pci_descriptor_t d;
                    d.bus = bus, d.device = device, d.function = function, d.header = functionHeader;

                    // put it in our list of pci functions (overflows at 372 descriptors thus we don't have to check anything since it's unlikely to have so many pci functions)
                    pciFuncs[pciIndex++] = d;
                }
            }
        }
    }
#endif
}

acpi_pci_descriptor_t *pciGetFunctions()
{
    return pciFuncs;
}

uint64_t pciGetFunctionsNum()
{
    return pciIndex;
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

// reboot using acpi
void acpiReboot()
{
    logError("ACPI Reboot unsupported. Trying fallback.");
    rebootFallback();
    hang();
}

// shutdown using acpi
void acpiShutdown()
{
    logError("ACPI Shutdown unsupported. Rebooting instead.");
    acpiReboot();
    hang();
}

// initialize the acpi subsystem
void acpiInit()
{
    // get rsdp
    rsdp = (acpi_rsdp_hdr_t *)bootloaderGetRSDP();

    vmmMap(vmmGetBaseTable(), rsdp, (void *)((uint64_t)rsdp - (uint64_t)bootloaderGetHHDM()), VMM_ENTRY_RW); // properly map the rsdp

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

    // enumerate PCI bus if MCFG is present
    if (mcfg)
    {
        pciFuncs = pmmPage(); // allocate a buffer hold the functions
        acpiEnumeratePCI();   // do the enumeration

        logInfo("acpi: detected %d pci functions", pciIndex);
    }
    else
    {
        logError("acpi: failed to enumerate pci bus");
    }
#endif
}