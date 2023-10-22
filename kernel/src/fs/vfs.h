#pragma once
#include <misc/utils.h>

typedef struct
{
    // metadata
    char *name;

    // tree links
    //    vertical (deep)
    void *parent;
    void *child;

    //    horizontal (long)
    void *prev;
    void *next;
} vfs_node_t;

void vfsInit();