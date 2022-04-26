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

struct pack acpi_gas
{
    uint8_t addressSpace;
    uint8_t bitWidth;
    uint8_t bitOffset;
    uint8_t accessSize;
    uint64_t address;
};

struct pack acpi_fadt
{
    struct acpi_sdt header;
    uint32_t FACS;
    uint32_t DSDT;
    uint8_t reserved;
    uint8_t prefferedPowerMode;
    uint16_t sciIntrerrupt;
    uint32_t smiCommand;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t s4bios;
    uint8_t pstate;
    uint32_t PM1aEvent;
    uint32_t PM1bEvent;
    uint32_t PM1aControl;
    uint32_t PM1BControl;
    uint32_t PM2Control;
    uint32_t PMTimer;
    uint32_t GPE0;
    uint32_t GPE1;
    uint8_t PM1EventLen;
    uint8_t PM1ControlLen;
    uint8_t PM2ControlLen;
    uint8_t PMTimerLen;
    uint8_t GPE0Len;
    uint8_t GPE1Len;
    uint8_t GPE1Base;
    uint8_t cStateControl;
    uint16_t worstC2;
    uint16_t worstC3;
    uint16_t flushSize;
    uint16_t flushPitch;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
    uint16_t bootFlags;
    uint8_t reserved2;
    uint32_t flags;
    struct acpi_gas reset;
    uint8_t resetValue;
    uint8_t reserved3[3];
    uint64_t FACS64;
    uint64_t DSDT64;
    struct acpi_gas PM1aEvent64;
    struct acpi_gas PM1bEvent64;
    struct acpi_gas PM1aControl64;
    struct acpi_gas PM1BControl64;
    struct acpi_gas PM2Control64;
    struct acpi_gas PMTimer64;
    struct acpi_gas GPE064;
    struct acpi_gas GPE164;
};

struct pack acpi_pci_config
{
    uint64_t base;
    uint16_t pciSegment;
    uint8_t startBus;
    uint8_t endBus;
    uint32_t reserved;
};

struct pack acpi_mcfg
{
    struct acpi_sdt header;
    uint64_t reserved;
    struct acpi_pci_config buses[];
};

struct pack acpi_pci_header
{
    uint16_t vendor;
    uint16_t device;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t programInterface;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
};

struct pack acpi_pci_descriptor
{
    struct acpi_pci_header *header;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

struct acpi_sdt *acpiGet(const char *sig);
void acpiEnumeratePCI();
void acpiInit();