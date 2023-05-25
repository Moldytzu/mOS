#pragma once
#include <misc/utils.h>

#define VFS_SECTOR 512
#define VFS_MBR_SIGNATURE 0xAA55

struct vfs_node_t;

pstruct
{
    uint8_t attributes;
    uint16_t chs1;
    uint8_t chs2;
    uint8_t type;
    uint16_t chs3;
    uint8_t chs4;
    uint32_t startSector;
    uint32_t sectors;
}
vfs_mbr_partition_t;

pstruct
{
    uint8_t bootstrap[440];
    uint32_t diskID;
    uint16_t reserved;
    vfs_mbr_partition_t partitions[4];
    uint16_t signature;
}
vfs_mbr_t;

pstruct
{
    uint32_t startLBA; // starting
    uint32_t endLBA;   // and ending sectors
    uint32_t sectors;
}
vfs_partition_t;

pstruct
{
    const char *interface;
    const char *friendlyName;
    uint32_t sectors;
    void (*read)(void *, uint64_t, uint64_t);  // buffer, sector, count
    void (*write)(void *, uint64_t, uint64_t); // buffer, sector, count

    vfs_partition_t partitions[4]; // for mbr are enough (TODO: support gpt!!!!)
}
vfs_drive_t;

pstruct
{
    void *context;         // filesystem dependent context
    const char *name;      // friendly name of the filesystem
    const char *mountName; // mount point
    uint8_t (*open)(struct vfs_node_t *);
    void (*close)(struct vfs_node_t *);
    void (*read)(struct vfs_node_t *, void *, uint64_t, uint64_t); // node, buffer, size, offset
    void (*write)(struct vfs_node_t *, void *, uint64_t, uint64_t); // node, buffer, size, offset
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
struct vfs_node_t *vfsAdd(struct vfs_node_t node);
void vfsRemove(struct vfs_node_t *node);
void vfsGetPath(uint64_t fd, void *buffer);
bool vfsExists(const char *name);
struct vfs_node_t *vfsNodes();
void vfsAddDrive(vfs_drive_t drive);
vfs_drive_t *vfsGetDrives();
bool vfsCheckMBR(vfs_mbr_t *sector);