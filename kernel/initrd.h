#pragma once
#include <utils.h>

struct pack dsfs_header
{
    const char signature[2];
    uint16_t version;
    uint32_t entries;
};

struct pack dsfs_entry
{
    const char name[56];
    uint64_t size;
};

void initrdInit();