#pragma once
#include <misc/utils.h>

#define SATA_SIG_ATA 0x00000101   // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101    // Port multiplier

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000

#define ATA_CMD_READ_DMA_EX 0x25

pstruct
{
    uint16_t VendorID;
    uint16_t DeviceID;
    uint16_t Command;
    uint16_t Status;
    uint8_t RevisionID;
    uint8_t ProgramInterface;
    uint8_t Subclass;
    uint8_t Class;
    uint8_t CacheLineSize;
    uint8_t LatencyTimer;
    uint8_t HeaderType;
    uint8_t BIST;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t CardBusCISPtr;
    uint16_t SubsystemVendorID;
    uint16_t SubsystemID;
    uint16_t ExpansionRomBaseAddr;
    uint16_t CapabilitiesPtr;
    uint16_t Rsv0;
    uint16_t Rsv1;
    uint16_t Rsv2;
    uint8_t IntreruptLine;
    uint8_t IntreruptPin;
    uint8_t MinGrant;
    uint8_t MaxLatency;
}
drv_pci_header0_t;
typedef enum
{
    FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
    FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
    FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA = 0x46,      // Data FIS - bidirectional
    FIS_TYPE_BIST = 0x58,      // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} ahci_fis_type_t;

typedef volatile struct
{
    uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
    uint32_t clbu;      // 0x04, command list base address upper 32 bits
    uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
    uint32_t is;        // 0x10, interrupt status
    uint32_t ie;        // 0x14, interrupt enable
    uint32_t cmd;       // 0x18, command and status
    uint32_t rsv0;      // 0x1C, Reserved
    uint32_t tfd;       // 0x20, task file data
    uint32_t sig;       // 0x24, signature
    uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;      // 0x30, SATA error (SCR1:SError)
    uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
    uint32_t ci;        // 0x38, command issue
    uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;       // 0x40, FIS-based switch control
    uint32_t rsv1[11];  // 0x44 ~ 0x6F, Reserved
    uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} ahci_port_t;

typedef volatile struct
{
    // 0x00 - 0x2B, Generic Host Control
    uint32_t cap;     // 0x00, Host capability
    uint32_t ghc;     // 0x04, Global host control
    uint32_t is;      // 0x08, Interrupt status
    uint32_t pi;      // 0x0C, Port implemented
    uint32_t vs;      // 0x10, Version
    uint32_t ccc_ctl; // 0x14, Command completion coalescing control
    uint32_t ccc_pts; // 0x18, Command completion coalescing ports
    uint32_t em_loc;  // 0x1C, Enclosure management location
    uint32_t em_ctl;  // 0x20, Enclosure management control
    uint32_t cap2;    // 0x24, Host capabilities extended
    uint32_t bohc;    // 0x28, BIOS/OS handoff control and status

    // 0x2C - 0x9F, Reserved
    uint8_t rsv[0xA0 - 0x2C];

    // 0xA0 - 0xFF, Vendor specific registers
    uint8_t vendor[0x100 - 0xA0];

    // 0x100 - 0x10FF, Port control registers
    ahci_port_t ports[1]; // 1 ~ 32
} ahci_hba_mem_t;

typedef struct
{
    // DWORD 0
    uint8_t fis_type; // FIS_TYPE_REG_H2D

    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 3;   // Reserved
    uint8_t c : 1;      // 1: Command, 0: Control

    uint8_t command;  // Command register
    uint8_t featurel; // Feature register, 7:0

    // DWORD 1
    uint8_t lba0;   // LBA low register, 7:0
    uint8_t lba1;   // LBA mid register, 15:8
    uint8_t lba2;   // LBA high register, 23:16
    uint8_t device; // Device register

    // DWORD 2
    uint8_t lba3;     // LBA register, 31:24
    uint8_t lba4;     // LBA register, 39:32
    uint8_t lba5;     // LBA register, 47:40
    uint8_t featureh; // Feature register, 15:8

    // DWORD 3
    uint8_t countl;  // Count register, 7:0
    uint8_t counth;  // Count register, 15:8
    uint8_t icc;     // Isochronous command completion
    uint8_t control; // Control register

    // DWORD 4
    uint8_t rsv1[4]; // Reserved
} ahci_fis_h2d_t;

typedef struct
{
    // DW0
    uint8_t cfl : 5; // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a : 1;   // ATAPI
    uint8_t w : 1;   // Write, 1: H2D, 0: D2H
    uint8_t p : 1;   // Prefetchable

    uint8_t r : 1;    // Reset
    uint8_t b : 1;    // BIST
    uint8_t c : 1;    // Clear busy upon R_OK
    uint8_t rsv0 : 1; // Reserved
    uint8_t pmp : 4;  // Port multiplier port

    uint16_t prdtl; // Physical region descriptor table length in entries

    // DW1
    volatile uint32_t prdbc; // Physical region descriptor byte count transferred

    // DW2, 3
    uint32_t ctba;  // Command table descriptor base address
    uint32_t ctbau; // Command table descriptor base address upper 32 bits

    // DW4 - 7
    uint32_t rsv1[4]; // Reserved
} ahci_command_header_t;
typedef struct
{
    uint32_t dba;  // Data base address
    uint32_t dbau; // Data base address upper 32 bits
    uint32_t rsv0; // Reserved

    // DW3
    uint32_t dbc : 22; // Byte count, 4M max
    uint32_t rsv1 : 9; // Reserved
    uint32_t i : 1;    // Interrupt on completion
} ahci_prdt_t;
typedef struct
{
    // 0x00
    uint8_t cfis[64]; // Command FIS

    // 0x40
    uint8_t acmd[16]; // ATAPI command, 12 or 16 bytes

    // 0x50
    uint8_t rsv[48]; // Reserved

    // 0x80
    ahci_prdt_t prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} ahci_tbl_t;

void ahciInit();