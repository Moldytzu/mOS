#pragma once
#include <utils.h>

struct vfs_node;

struct vfs_fs
{
    const char *name;      // friendly name of the filesystem
    const char *mountName; // mount point
    uint8_t (*open)(struct vfs_node *);
    void (*close)(struct vfs_node *);
    void (*read)(struct vfs_node *, void *, uint64_t, uint64_t);
    void (*write)(struct vfs_node *, void *, uint64_t, uint64_t);
};

struct vfs_node
{
    struct vfs_node *parent;   // parent node
    struct vfs_fs *filesystem; // filesystem interface
    uint64_t size;             // size of the contents
    uint64_t id;               // ID
    uint8_t path[128];         // path/name

    struct vfs_node *next; // next node
};

void vfsInit();
uint64_t vfsOpen(const char *name);
uint64_t vfsSize(uint64_t fd);
void vfsClose(uint64_t fd);
void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsAdd(struct vfs_node node);
void vfsRemove(struct vfs_node *node);
struct vfs_node *vfsNodes();