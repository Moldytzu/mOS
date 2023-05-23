#pragma once
#include <misc/utils.h>
#include <fs/vfs.h>

pstruct
{
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fats;
    uint16_t rootDirectoryEntries;
    uint16_t totalSectors16;
    uint8_t mediaDescriptorType;
    uint16_t reserved;
    uint16_t sectorsPerTrack;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    uint32_t sectorsPerFat;
    uint16_t flags;
    uint16_t version;
    uint32_t clusterNumber;
    uint16_t sectorNumberFSInfo;
    uint16_t backupBootSector;
    uint8_t reserved2[12];
    uint8_t driveNumber;
    uint8_t ntFlags;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t label[11];
    uint64_t identifier;
    uint8_t bootCode[420];
    uint16_t bootSignature;
} fat_bpb_t;

pstruct
{
    fat_bpb_t *bpb;
    vfs_drive_t *drive;
    size_t partition;
} fat_context_t;

bool fatIsValid(fat_bpb_t *bpb);
bool fatCreate(fat_bpb_t *bpb, vfs_drive_t *drive, size_t partition);