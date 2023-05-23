#include <fs/fat.h>
#include <fs/vfs.h>
#include <misc/logger.h>
#include <mm/pmm.h>

#define CONTEXT(x) ((fat_context_t *)(x))

bool fatIsValid(fat_bpb_t *bpb)
{
    return bpb->bootSignature == VFS_MBR_SIGNATURE;
}

void fatFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
}

bool fatCreate(fat_bpb_t *bpb, vfs_drive_t *drive, size_t partition)
{
    if (!fatIsValid(bpb))
        return false;

    // create a vfs-compatible context
    vfs_fs_t *filesystem = pmmPage();
    filesystem->context = pmmPage();

    // create an internal context
    CONTEXT(filesystem->context)->bpb = bpb;
    CONTEXT(filesystem->context)->drive = drive;
    CONTEXT(filesystem->context)->partition = partition;

    // generate a mount name
    char *mountName = pmmPage();
    const char *partStr = to_string(partition);

    // sprintf("%s%sp%d")
    memcpy(mountName, "/", 1);
    memcpy(mountName + 1, drive->interface, strlen(drive->interface));
    memcpy(mountName + strlen(drive->interface) + 1, drive->friendlyName, strlen(drive->friendlyName));
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + 1, "p", 1);
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + 2, partStr, strlen(partStr));
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + strlen(partStr) + 2, "/", 1);

    logDbg(LOG_SERIAL_ONLY, "fat: mounting filesystem as %s", mountName);

    filesystem->name = "fat";
    filesystem->mountName = mountName;

    // set handlers
    filesystem->read = fatFSRead;

    // create root node
    struct vfs_node_t rootNode;
    zero(&rootNode, sizeof(rootNode));
    rootNode.filesystem = filesystem;
    vfsAdd(rootNode);

    return true;
}