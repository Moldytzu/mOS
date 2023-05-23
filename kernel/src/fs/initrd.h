#pragma once
#include <misc/utils.h>

pstruct
{
    uint8_t signature[2];
    uint16_t version;
    uint32_t entries;
}
dsfs_header_t;

pstruct
{
    uint8_t name[56];
    uint64_t size;
}
dsfs_entry_t;

pstruct
{
    dsfs_header_t header;
    dsfs_entry_t firstEntry;
}
dsfs_fs_t;

void initrdInit();
void initrdMount();
void *initrdGet(const char *name);