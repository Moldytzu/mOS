#pragma once
#include <utils.h>
#include <lai/core.h>
#include <lai/host.h>
#include <lai/helpers/pci.h>
#include <lai/helpers/pm.h>
#include <lai/helpers/sci.h>
#include <lai/helpers/resource.h>
#include <lai/helpers/pc-bios.h>

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
    uint8_t oem;
    uint64_t oemTableID;
    uint32_t oemRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};

void acpiInit();