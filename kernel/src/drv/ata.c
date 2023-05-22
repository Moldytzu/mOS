#include <drv/ata.h>
#include <cpu/io.h>
#include <misc/logger.h>
#include <mm/pmm.h>
#include <fs/vfs.h>

// constants
#define ATA_SECTOR 512

// I/O ports
#define ATA_DATA 0x1F0
#define ATA_ERROR 0x1F1
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_SECTOR_NUMBER 0x1F3
#define ATA_CYL_LOW 0x1F4
#define ATA_CYL_HIGH 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_STATUS 0x1F7
#define ATA_COMMAND 0x1F7

// drives
#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE 0xB0

// commands
#define ATA_CMD_IDENTIFY_PIO 0xEC
#define ATA_CMD_READ_SECTORS_EXT_PIO 0x24

// status flags
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERR 0x01

bool presentDrives[2];

uint16_t driveID[2][256]; // 2 drives, 256 words

// selects drive
void ataSelect(uint8_t drive)
{
    outb(ATA_DRIVE, drive);
}

// issues a command
void ataCommand(uint8_t command)
{
    outb(ATA_COMMAND, command);
}

// returns status of bus
uint8_t ataStatus()
{
    return inb(ATA_STATUS);
}

// performs a 48 bit pio lba read
bool ataRead(uint8_t drive, void *buffer, uint64_t sector, uint16_t sectorCount)
{
    if (drive == ATA_DRIVE_MASTER)
        ataSelect(0x40);
    else
        ataSelect(0x50);

    outb(ATA_SECTOR_COUNT, sector >> 8);        // send high byte of the sector count
    outb(ATA_SECTOR_NUMBER, sector >> (8 * 4)); // send 4th byte of lba
    outb(ATA_CYL_LOW, sector >> (8 * 5));       // send 5th byte of lba
    outb(ATA_CYL_HIGH, sector >> (8 * 6));      // send 6th byte of lba
    outb(ATA_SECTOR_COUNT, sector);             // send low byte of the sector count
    outb(ATA_SECTOR_NUMBER, sector >> (8 * 1)); // send 1st byte of lba
    outb(ATA_CYL_LOW, sector >> (8 * 2));       // send 2nd byte of lba
    outb(ATA_CYL_HIGH, sector >> (8 * 3));      // send 3rd byte of lba

    ataCommand(ATA_CMD_READ_SECTORS_EXT_PIO); // send the read command

    uint16_t *words = (uint16_t *)buffer;

    for (int i = 0; i < sectorCount; i++)
    {
        while (ataStatus() & ATA_STATUS_BSY) // wait for drive to process our command
        {
            if (ataStatus() & ATA_STATUS_DRQ)
                break;

            if (ataStatus() & ATA_STATUS_ERR) // read error
                return false;
        }

        for (size_t i = 0; i < ATA_SECTOR / sizeof(uint16_t); i++) // read data
            *(words++) = inw(ATA_DATA);
    }

    return true;
}

void ataVFSReadMaster(void *buffer, uint64_t sector, uint64_t count)
{
    ataRead(ATA_DRIVE_MASTER, buffer, sector, count);
}

void ataVFSReadSlave(void *buffer, uint64_t sector, uint64_t count)
{
    ataRead(ATA_DRIVE_SLAVE, buffer, sector, count);
}

// gets sectors of the drive
uint64_t ataSectors(uint8_t drive)
{
    uint64_t *sectors = (uint64_t *)&driveID[drive - ATA_DRIVE_MASTER][100];
    return *sectors;
}

// sends identify to drive (returns 1 on success)
bool ataIdentify(uint8_t drive)
{
    ataSelect(drive);

    // zero lba
    for (int i = 0; i < 3; i++)
        outb(0x1F2 + i, 0);

    ataCommand(ATA_CMD_IDENTIFY_PIO);

    if (ataStatus() == 0) // non-present drive
        return 0;

    while (ataStatus() & ATA_STATUS_BSY) // wait for drive to not be busy
    {
        if (ataStatus() & ATA_STATUS_DRQ)
            break;

        if (ataStatus() & ATA_STATUS_ERR) // faulty drive
            return 0;
    }

    for (int i = 0; i < 256; i++) // read information
        driveID[drive - ATA_DRIVE_MASTER][i] = inw(ATA_DATA);

    return 1;
}

// check for presence of drive
bool ataIsDrivePresent(uint8_t drive)
{
    ataSelect(drive);

    if (ataStatus() == 0xFF) // non-present bus
        return false;

    return ataIdentify(drive);
}

void ataInit()
{
    presentDrives[0] = ataIsDrivePresent(ATA_DRIVE_MASTER);
    presentDrives[1] = ataIsDrivePresent(ATA_DRIVE_SLAVE);

    logInfo("ata: detected %d drives", presentDrives[0] + presentDrives[1]);

    if (!(presentDrives[0] + presentDrives[1]))
        return;

    if (presentDrives[0] && !(driveID[0][83] & 0x400 /*bit 10*/)) // drive doesn't support LBA48 mode
        presentDrives[0] = false;                                 // mark as non-present because it doesn't support what we want

    if (presentDrives[1] && !(driveID[1][83] & 0x400 /*bit 10*/)) // drive doesn't support LBA48 mode
        presentDrives[1] = false;                                 // mark as non-present because it doesn't support what we want

    if (presentDrives[0])
    {
        size_t sectors = ataSectors(ATA_DRIVE_MASTER);
        logInfo("ata: drive 0 has %d sectors (%d MB)", sectors, (sectors * ATA_SECTOR) / 1024 / 1024);

        // register in vfs the drive
        vfs_drive_t drive;
        zero(&drive, sizeof(drive));
        drive.interface = "ata";
        drive.friendlyName = "master";
        drive.sectors = sectors;
        drive.read = ataVFSReadMaster;

        vfs_mbr_t firstSector;
        ataRead(ATA_DRIVE_MASTER, &firstSector, 0, 1);

        if(vfsCheckMBR(&firstSector)) // check if mbr is valid 
        {
            int part = 0;
            for(int i = 0; i < 4; i++) // parse all the partitions
            {
                vfs_mbr_partition_t *partition = &firstSector.partitions[i];
                if(!partition->startSector)
                    continue;

                vfs_partition_t *vfsPart = &drive.partitions[part++];
                vfsPart->startLBA = partition->startSector;
                vfsPart->endLBA = partition->startSector + partition->sectors;
                vfsPart->sectors = partition->sectors;
            }
        }

        vfsAddDrive(drive);
    }

    if (presentDrives[1])
        logInfo("ata: drive 0 has %d sectors (%d MB)", ataSectors(ATA_DRIVE_SLAVE), (ataSectors(ATA_DRIVE_SLAVE) * ATA_SECTOR) / 1024 / 1024);
}