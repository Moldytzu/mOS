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
    uint32_t rootClusterNumber;
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
    unsigned readOnly : 1;
    unsigned hidden : 1;
    unsigned system : 1;
    unsigned volumeID : 1;
    unsigned directory : 1;
    unsigned archive : 1;
    unsigned reserved : 2;
} fat_attributes_t;

pstruct
{
    char name[11];
    fat_attributes_t attributes;
    uint8_t flags;
    uint8_t timeResolution;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t lastAccessDate;
    uint16_t clusterHigh;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t clusterLow;
    uint32_t size;
} fat_dir_t;

pstruct
{
    fat_bpb_t *bpb;
    vfs_drive_t *drive;
    size_t partition;
} fat_context_t;

bool fatIsValid(fat_bpb_t *bpb);
bool fatCreate(fat_bpb_t *bpb, vfs_drive_t *drive, size_t partition);