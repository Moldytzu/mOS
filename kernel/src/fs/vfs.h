#pragma once
#include <misc/utils.h>

#define VFS_PATH_DELIMITER '/'

#define VFS_FLAG_DIRECTORY 0b1

typedef struct
{
    // metadata
    char *name;
    uint16_t flags;

    // tree links
    //    vertical (deep)
    void *parent;
    void *child;

    //    horizontal (long)
    void *prev;
    void *next;
} vfs_node_t;

void vfsInit();