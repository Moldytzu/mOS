#pragma once
#include <utils.h>

struct vfs_fs
{
    const char *name;
    const char *mountName;
    void *handlers[5]; // open, close, read, write
};

struct vfs_node
{
    struct vfs_node *parent;
    struct vfs_fs *filesystem;
    uint8_t path[128];
};

void vfsInit();
void vfsAdd(struct vfs_node node);
void vfsRemove(struct vfs_node *node);
struct vfs_node *vfsNodes();