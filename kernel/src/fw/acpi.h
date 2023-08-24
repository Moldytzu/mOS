#pragma once
#include <misc/utils.h>

#define ACPI_GAS_ACCESS_MEMORY 0
#define ACPI_GAS_ACCESS_IO 1

#define ACPI_ENABLED 1

#define ACPI_PM1_CONTROL_SLP_EN (1 << 13)
#define ACPI_PM1_CONTROL_SLP_TYPx (1 << 10)

#define ACPI_BUTTON_POWER (1 << 8)
#define ACPI_BUTTON_SLEEP (1 << 9)

#define ACPI_FADT_RESET_REG_SUP (1 << 10)

pstruct
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
}
acpi_rsdp_t;

pstruct
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
}
acpi_sdt_t;

pstruct
{
    acpi_sdt_t header;
    uint32_t entries[];
}
acpi_rsdt_t;

pstruct
{
    acpi_sdt_t header;
    uint64_t entries[];
}
acpi_xsdt_t;

pstruct
{
    uint8_t addressSpace;
    uint8_t bitWidth;
    uint8_t bitOffset;
    uint8_t accessSize;
    uint64_t address;
}
acpi_gas_t;

pstruct
{
    acpi_sdt_t header;
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
    acpi_gas_t reset;
    uint8_t resetValue;
    uint8_t reserved3[3];
    uint64_t FACS64;
    uint64_t DSDT64;
    acpi_gas_t PM1aEvent64;
    acpi_gas_t PM1bEvent64;
    acpi_gas_t PM1aControl64;
    acpi_gas_t PM1BControl64;
    acpi_gas_t PM2Control64;
    acpi_gas_t PMTimer64;
    acpi_gas_t GPE064;
    acpi_gas_t GPE164;
}
acpi_fadt_t;

pstruct
{
    uint64_t base;
    uint16_t pciSegment;
    uint8_t startBus;
    uint8_t endBus;
    uint32_t reserved;
}
acpi_pci_config_t;

pstruct
{
    acpi_sdt_t header;
    uint64_t reserved;
    acpi_pci_config_t buses[];
}
acpi_mcfg_t;

pstruct
{
    acpi_sdt_t header;
    uint8_t revision;
    uint8_t comparatorCount : 5;
    uint8_t counterSize : 1;
    uint8_t reserved : 1;
    uint8_t replaceLegacy : 1;
    uint16_t pciVendor;
    acpi_gas_t base;
    uint8_t hpetNumber;
    uint16_t minimumTick;
    uint8_t pageProtection;
}
acpi_hpet_t;

pstruct
{
    acpi_sdt_t header;
    uint32_t lapicAddress;
    uint32_t flags;
}
acpi_madt_t;

pstruct
{
    acpi_sdt_t header;
    uint8_t aml[];
}
acpi_dsdt_t;

// implemented in fw/acpi/tables.c
extern acpi_rsdp_t *rsdp;
extern acpi_xsdt_t *sdt;
extern acpi_mcfg_t *mcfg;
extern acpi_fadt_t *fadt;
extern acpi_dsdt_t *dsdt;

// tables
acpi_sdt_t *acpiGet(const char *sig, int index);

// power
void acpiInit();
void acpiReboot();
void acpiShutdown();

// aml
void amlInit();
bool amlEnterSleepState(uint8_t state);

// sci
uint32_t acpiGetSCIEvent();