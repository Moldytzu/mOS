#include <fw/acpi.h>
#include <fw/bootloader.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <cpu/io.h>
#include <main/panic.h>

uint8_t revision;
struct acpi_rsdp *rsdp;
struct acpi_sdt *sdt;
struct acpi_fadt *fadt;
struct acpi_mcfg *mcfg;
struct acpi_hpet *hpet;

struct acpi_pci_descriptor pciFuncs[8 * 32 * 255]; // 8 functions / device, 32 devices / bus, 255 buses
uint16_t pciIndex = 0;

// get a descriptor table with a signature
struct acpi_sdt *acpiGet(const char *sig)
{
    bool xsdt = sdt->signature[0] == 'X'; // XSDT's signature is XSDT, RSDT's signature is RSDT
    size_t entries = sdt->length - sizeof(struct acpi_sdt);

    if (xsdt)
    { // xsdt parsing
        struct acpi_xsdt *root = (struct acpi_xsdt *)sdt;
        for (size_t i = 0; i < entries / sizeof(uint64_t); i++)
        {
            struct acpi_sdt *table = (struct acpi_sdt *)root->entries[i]; // every entry in the table is an address to another table
#ifdef K_ACPI_DEBUG
            printks("acpi: %p %c%c%c%c and %c%c%c%c\n\r", table, table->signature[0], table->signature[1], table->signature[2], table->signature[3], sig[0], sig[1], sig[2], sig[3]);
#endif
            if (memcmp8((void *)sig, table->signature, 4) == 0) // compare the signatures
                return table;
        }
    }
    else
    { // rsdp parsing
        struct acpi_rsdt *root = (struct acpi_rsdt *)sdt;
        for (size_t i = 0; i < entries / sizeof(uint32_t); i++)
        {
            struct acpi_sdt *table = (struct acpi_sdt *)root->entries[i]; // every entry in the table is an address to another table
#ifdef K_ACPI_DEBUG
            printks("acpi: %p %c%c%c%c and %c%c%c%c\n\r", table, table->signature[0], table->signature[1], table->signature[2], table->signature[3], sig[0], sig[1], sig[2], sig[3]);
#endif
            if (memcmp8((void *)sig, table->signature, 4) == 0) // compare the signatures
                return table;
        }
    }

    return NULL; // return nothing
}

// enumerate a function
void enumerateFunction(uint64_t base, uint8_t function, uint8_t device, uint8_t bus)
{
    struct acpi_pci_header *header = (struct acpi_pci_header *)(base + (function << 12));

    if (header->device == UINT16_MAX || header->device == 0) // invalid function
        return;

#ifdef K_ACPI_DEBUG
    printks("acpi: found pci function %x:%x at %d.%d.%d\n\r", header->vendor, header->device, bus, device, function);
#endif

    // build the descriptor
    struct acpi_pci_descriptor d;
    d.bus = bus, d.device = device, d.function = function, d.header = header;

    // put it in our list of pci functions
    pciFuncs[pciIndex++] = d;
}

// enumerate a device
void enumerateDevice(uint64_t base, uint8_t device, uint8_t bus)
{
    struct acpi_pci_header *header = (struct acpi_pci_header *)(base + (device << 15));

    if (header->device == UINT16_MAX || header->device == 0) // invalid device
        return;

    for (int i = 0; i < 8; i++) // max 8 functions/device
        enumerateFunction((uint64_t)header, i, device, bus);
}

// enumerate a bus
void enumerateBus(uint64_t base, uint8_t bus)
{
    struct acpi_pci_header *header = (struct acpi_pci_header *)(base + (bus << 20));

    if (header->device == UINT16_MAX || header->device == 0) // invalid bus
        return;

    for (int i = 0; i < 32; i++) // max 32 devices/bus
        enumerateDevice((uint64_t)header, i, bus);
}

// enumerate the pci bus using mcfg
void acpiEnumeratePCI()
{
    size_t entries = (mcfg->header.length - sizeof(struct acpi_mcfg)) / sizeof(struct acpi_pci_config);
    for (int i = 0; i < entries; i++)
        for (int j = mcfg->buses[i].startBus; j < mcfg->buses[i].endBus; j++)
            enumerateBus(mcfg->buses[i].base, j); // enumerate every bus
}

// reboot using acpi
void acpiReboot()
{
    if (revision == 0 || !fadt) // acpi 1.0 doesn't support reboot, fadt must be present for acpi 2.0+ reboot
        goto triplefault;

#ifdef K_ACPI_DEBUG
    printks("acpi: performing acpi reboot...\n\r");
#endif

    switch (fadt->reset.addressSpace)
    {
    case ACPI_GAS_SYSIO: // if the reset register is i/o mapped
        outb(fadt->reset.address, fadt->resetValue);
        break;
    case ACPI_GAS_SYSMEM: // if the reset register is in the system memory
        *(uint8_t *)fadt->reset.address = fadt->resetValue;
    default:
        break;
    }

triplefault:
#ifdef K_ACPI_DEBUG
    printks("acpi: reboot unsupported. triple faulting.\n\r");
#endif
    iasm("lidt %0" ::"m"(pciFuncs)); // load an invalid IDT => triple fault / reboot
}

// initialize the acpi subsystem
void acpiInit()
{
    // get rsdp
    rsdp = (struct acpi_rsdp *)(void *)bootloaderGetRSDP()->rsdp;

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
        struct acpi_rsdt *root = (struct acpi_rsdt *)sdt;
        size_t entries = (sdt->length - sizeof(struct acpi_sdt)) / sizeof(uint32_t);
        for (size_t i = 0; i < entries; i++)
        {
            struct acpi_sdt *table = (struct acpi_sdt *)root->entries[i]; // every entry in the table is an address to another table
            printks("acpi: found %c%c%c%c\n\r", table->signature[0], table->signature[1], table->signature[2], table->signature[3]);
        }
    }
    else
    {
        struct acpi_xsdt *root = (struct acpi_xsdt *)sdt;
        size_t entries = (root->header.length - sizeof(struct acpi_sdt)) / sizeof(uint64_t);
        for (size_t i = 0; i < entries; i++)
        {
            struct acpi_sdt *table = (struct acpi_sdt *)root->entries[i]; // every entry in the table is an address to another table
            printks("acpi: found %c%c%c%c\n\r", table->signature[0], table->signature[1], table->signature[2], table->signature[3]);
        }
    }
#endif

    // get fadt & mcfg
    fadt = (struct acpi_fadt *)acpiGet("FACP");
    mcfg = (struct acpi_mcfg *)acpiGet("MCFG");

    // enable ACPI mode if FADT is present
    if (fadt)
        outb(fadt->smiCommand, fadt->acpiEnable);

    // enumerate PCI bus if MCFG is present
    if (mcfg)
        acpiEnumeratePCI();

#ifdef K_ACPI_DEBUG
    printks("acpi: found %d pci functions in total\n\r", pciIndex);
#endif
}