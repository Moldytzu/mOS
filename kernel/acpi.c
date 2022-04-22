#include <acpi.h>
#include <heap.h>
#include <vmm.h>
#include <bootloader.h>

uint8_t revision;
struct acpi_rsdp *rsdp;
struct acpi_sdt *sdt;

struct acpi_sdt *acpiGet(const char *sig)
{
    bool xsdt = sdt->signature[0] == 'X'; // XSDT's signature is XSDT, RSDT's signature is RSDT

    if (xsdt)
    { // xsdt parsing
        struct acpi_xsdt *root = (struct acpi_xsdt *)sdt;
        size_t entries = (root->header.length - sizeof(struct acpi_sdt)) / sizeof(uint64_t);
        for (size_t i = 0; i < entries; i++)
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
        size_t entries = (sdt->length - sizeof(struct acpi_sdt)) / sizeof(uint32_t);
        for (size_t i = 0; i < entries; i++)
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
}