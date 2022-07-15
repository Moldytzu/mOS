#pragma once
#include <misc/utils.h>

struct pack dsfs_header
{
    uint8_t signature[2];
    uint16_t version;
    uint32_t entries;
};

struct pack dsfs_entry
{
    uint8_t name[56];
    uint64_t size;
};

struct pack dsfs_fs
{
    struct dsfs_header header;
    struct dsfs_entry firstEntry;
};

void initrdInit();
void initrdMount();
void *initrdGet(const char *name);