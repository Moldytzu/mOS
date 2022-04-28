#pragma once
#include <utils.h>

struct vfs_node;

struct vfs_fs
{
    const char *name;
    const char *mountName;
    uint8_t (*open)(struct vfs_node *);
    void (*close)(struct vfs_node *);
    void (*read)(struct vfs_node *, void *, uint64_t, uint64_t);
    void (*write)(struct vfs_node *, void *, uint64_t, uint64_t);
};

struct vfs_node
{
    struct vfs_node *parent;
    struct vfs_fs *filesystem;
    uint64_t size;
    uint8_t path[128];
};

void vfsInit();
uint64_t vfsOpen(const char *name);
void vfsClose(uint64_t fd);
void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsAdd(struct vfs_node node);
void vfsRemove(struct vfs_node *node);
struct vfs_node *vfsNodes();