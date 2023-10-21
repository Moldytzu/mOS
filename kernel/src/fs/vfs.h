#pragma once
#include <misc/utils.h>

typedef struct
{
    char *name;

    // vertical (deep)
    void *parent;
    void *child;

    // horizontal (long)
    void *prev;
    void *next;
} vfs_node_t;

void vfsInit();