#pragma once
#include <utils.h>

struct pack acpi_rsdp
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem[6];
    uint8_t version;
    uint32_t rsdt;
    uint32_t length;
    uint64_t xsdt;
    uint8_t echecksum;
    uint8_t reserved[3];
};

struct pack acpi_sdt
{
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemID[6];
    uint8_t oemTableID[8];
    uint32_t oemRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};

struct pack acpi_rsdt
{
    struct acpi_sdt header;
    uint32_t entries[];
};

struct pack acpi_xsdt
{
    struct acpi_sdt header;
    uint64_t entries[];
};

struct acpi_sdt *acpiGet(const char *sig);
void acpiInit();