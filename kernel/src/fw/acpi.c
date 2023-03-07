#include <fw/acpi.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <cpu/idt.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <lai/core.h>
#include <lai/host.h>
#include <lai/helpers/sci.h>
#include <lai/helpers/pm.h>

uint8_t revision;
acpi_rsdp_hdr_t *rsdp;
acpi_sdt_t *sdt;
acpi_fadt_hdr_t *fadt;
acpi_mcfg_t *mcfg;

acpi_pci_descriptor_t *pciFuncs = NULL;
uint16_t pciIndex = 0;

// get a descriptor table with a signature
acpi_sdt_t *acpiGet(const char *sig, int index)
{
    if (!sdt)
        return NULL;

    bool xsdt = sdt->signature[0] == 'X'; // XSDT's signature is XSDT, RSDT's signature is RSDT
    size_t entries = sdt->length - sizeof(acpi_sdt_t);

    if (xsdt)
    { // xsdt parsing
        acpi_xsdt_hdr_t *root = (acpi_xsdt_hdr_t *)sdt;
        for (size_t i = 0; i < entries / sizeof(uint64_t); i++)
        {
            acpi_sdt_t *table = (acpi_sdt_t *)root->entries[i]; // every entry in the table is an address to another table
#ifdef K_ACPI_DEBUG
            printks("acpi: %p %c%c%c%c and %c%c%c%c\n\r", table, table->signature[0], table->signature[1], table->signature[2], table->signature[3], sig[0], sig[1], sig[2], sig[3]);
#endif
            if (memcmp8((void *)sig, table->signature, 4) == 0 && index-- == 0) // compare the signatures
                return table;
        }
    }
    else
    { // rsdt parsing
        acpi_rsdt_hdr_t *root = (acpi_rsdt_hdr_t *)sdt;
        for (size_t i = 0; i < entries / sizeof(uint32_t); i++)
        {
            acpi_sdt_t *table = (acpi_sdt_t *)root->entries[i]; // every entry in the table is an address to another table
#ifdef K_ACPI_DEBUG
            printks("acpi: %p %c%c%c%c and %c%c%c%c\n\r", table, table->signature[0], table->signature[1], table->signature[2], table->signature[3], sig[0], sig[1], sig[2], sig[3]);
#endif
            if (memcmp8((void *)sig, table->signature, 4) == 0 && index-- == 0) // compare the signatures
                return table;
        }
    }

    return NULL; // return nothing
}

// enumerate the pci bus using mcfg
void acpiEnumeratePCI()
{
    if (!mcfg)
        return;

    size_t entries = (mcfg->header.length - sizeof(acpi_mcfg_t)) / sizeof(acpi_pci_config_t);
    for (int i = 0; i < entries; i++)
    {
        // enumerate each bus
        for (int bus = mcfg->buses[i].startBus; bus < mcfg->buses[i].endBus; bus++)
        {
            uint64_t base = mcfg->buses[i].base;

            acpi_pci_header_t *baseHeader = (acpi_pci_header_t *)base;

            vmmMap(vmmGetBaseTable(), baseHeader, baseHeader, false, true, true); // map the header

            // non-existent bus
            if (baseHeader->device == UINT16_MAX || baseHeader->device == 0)
                continue;

            // enumerate each device
            for (int device = 0; device < 32; device++)
            {
                // enumerate each function
                for (int function = 0; function < 8; function++)
                {
                    acpi_pci_header_t *header = (acpi_pci_header_t *)(base + (bus << 20 | device << 15 | function << 12));

                    vmmMap(vmmGetBaseTable(), header, header, false, true, true); // map the header

                    if (header->device == UINT16_MAX || header->device == 0) // invalid function
                        continue;

#ifdef K_ACPI_DEBUG
                    printks("acpi: found pci function %x:%x at %d.%d.%d\n\r", header->vendor, header->device, bus, device, function);
#endif

                    // build the descriptor
                    acpi_pci_descriptor_t d;
                    d.bus = bus, d.device = device, d.function = function, d.header = header;

                    // put it in our list of pci functions
                    if (pciIndex > 4096 / sizeof(acpi_pci_descriptor_t)) // very unlikely
                        panick("Can't hold that many PCI descriptors!");

                    pciFuncs[pciIndex++] = d;
                }
            }
        }
    }
}

acpi_pci_descriptor_t *pciGetFunctions()
{
    return pciFuncs;
}

uint64_t pciGetFunctionsNum()
{
    return pciIndex;
}

// reboot using acpi
void acpiReboot()
{
#ifdef K_ACPI_LAI
    lai_acpi_reset();
#else
    logError("Reboot unsupported. LAI support is disabled.");
    hang();
#endif
}

void acpiShutdown()
{
#ifdef K_ACPI_LAI
    lai_enter_sleep(5);
#else
    logError("Shutdown unsupported. LAI support is disabled.");
    hang();
#endif
}

// init lai
void laiInit()
{
#ifdef K_ACPI_LAI
    lai_set_acpi_revision(revision);
    lai_create_namespace();
    lai_enable_acpi(1); // use the lapic
#endif
}

// initialize the acpi subsystem
void acpiInit()
{
    // get rsdp
    rsdp = (acpi_rsdp_hdr_t *)bootloaderGetRSDP();

    vmmMap(vmmGetBaseTable(), rsdp, (void *)((uint64_t)rsdp - (uint64_t)bootloaderGetHHDM()), false, true, true); // properly map the rsdp

    // parse the version field
    revision = rsdp->version;

    // set the system descriptor table root based on the revision
    if (revision == 0)
        sdt = (void *)(uint64_t)rsdp->rsdt;
    else if (revision == 2)
        sdt = (void *)rsdp->xsdt;

#ifdef K_ACPI_DEBUG
    if (revision == 0)
    {
        acpi_rsdt_t *root = (acpi_rsdt_t *)sdt;
        size_t entries = (sdt->length - sizeof(acpi_sdt_t)) / sizeof(uint32_t);
        for (size_t i = 0; i < entries; i++)
        {
            acpi_sdt_t *table = (acpi_sdt_t *)root->entries[i]; // every entry in the table is an address to another table
            printks("acpi: found %c%c%c%c\n\r", table->signature[0], table->signature[1], table->signature[2], table->signature[3]);
        }
    }
    else
    {
        acpi_xsdt_t *root = (acpi_xsdt_t *)sdt;
        size_t entries = (root->header.length - sizeof(acpi_sdt_t)) / sizeof(uint64_t);
        for (size_t i = 0; i < entries; i++)
        {
            acpi_sdt_t *table = (acpi_sdt_t *)root->entries[i]; // every entry in the table is an address to another table
            printks("acpi: found %c%c%c%c\n\r", table->signature[0], table->signature[1], table->signature[2], table->signature[3]);
        }
    }
#endif

    laiInit();

    // get fadt & mcfg
    fadt = (acpi_fadt_hdr_t *)acpiGet("FACP", 0);
    mcfg = (acpi_mcfg_t *)acpiGet("MCFG", 0);

    // enable ACPI mode if FADT is present
    if (fadt)
        outb(fadt->smiCommand, fadt->acpiEnable);

    // enumerate PCI bus if MCFG is present
    if (mcfg)
    {
        pciFuncs = pmmPage(); // allocate a buffer hold the functions
        acpiEnumeratePCI();   // do the enumeration
    }

#ifdef K_ACPI_DEBUG
    printks("acpi: found %d pci functions in total\n\r", pciIndex);
#endif

    if (pciIndex)
        logInfo("acpi: detected %d pci functions", pciIndex);
    else
        logWarn("acpi: failed to detect pci functions! mcfg wasn't detected...");
}