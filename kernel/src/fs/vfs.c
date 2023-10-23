#include <fs/vfs.h>
#include <fs/fat.h>
#include <mm/blk.h>
#include <mm/pmm.h>
#include <misc/logger.h>

typedef uint64_t vfs_open_mode_t;
typedef uint64_t vfs_node_flags_t;

typedef struct
{
    void (*read)(void *buffer, uint64_t count, uint64_t offset);
    void (*write)(void *buffer, uint64_t count, uint64_t offset);
} vfs_node_ops_t;

typedef struct
{
    void *fsHandle;   // pointer to a vfs_fs_t
    void *fsSpecific; // pointer to a filesystem specific structure

    vfs_open_mode_t openMode;
    vfs_node_flags_t flags;

    size_t bytes;        // bytes occupied
    size_t bytesOnDrive; // bytes occupied on disk (allocated bytes on drive)

    vfs_node_ops_t ops;
} vfs_node_t;

typedef struct
{
    vfs_node_t (*open)(char *path, vfs_open_mode_t mode);
    void (*close)(vfs_node_t *node);
} vfs_fs_ops_t;

typedef struct
{
    char *mountName;
    vfs_fs_ops_t ops;
} vfs_fs_t;

void vfsInit()
{

    while (1)
        ;
}