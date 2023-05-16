#pragma once
#include <misc/utils.h>

struct vfs_node_t;

pstruct
{
    const char *name;      // friendly name of the filesystem
    const char *mountName; // mount point
    uint8_t (*open)(struct vfs_node_t *);
    void (*close)(struct vfs_node_t *);
    void (*read)(struct vfs_node_t *, void *, uint64_t, uint64_t);
    void (*write)(struct vfs_node_t *, void *, uint64_t, uint64_t);
}
vfs_fs_t;

struct vfs_node_t
{
    vfs_fs_t *filesystem; // filesystem interface
    uint64_t size;        // size of the contents
    uint64_t id;          // ID
    uint8_t path[128];    // path/name

    struct vfs_node_t *next; // next node
};

void vfsInit();
uint64_t vfsOpen(const char *name);
uint64_t vfsSize(uint64_t fd);
void vfsClose(uint64_t fd);
void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset);
void vfsAdd(struct vfs_node_t node);
void vfsRemove(struct vfs_node_t *node);
void vfsGetPath(uint64_t fd, void *buffer);
bool vfsExists(const char *name);
struct vfs_node_t *vfsNodes();