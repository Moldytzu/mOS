#include <fs/vfs.h>
#include <fs/fat.h>
#include <mm/blk.h>
#include <mm/pmm.h>
#include <misc/logger.h>

#define ISVALID(node) (node && node->filesystem)

struct vfs_node_t rootNode; // root of the linked list
uint64_t lastNode = 0;

vfs_fs_t rootFS;
vfs_drive_t drives[128];
size_t lastDrive = 0;

// initialize the subsystem
void vfsInit()
{
    zero(&rootFS, sizeof(rootFS)); // clear the root filesystem
    zero(drives, sizeof(drives));  // zero all drives

    // metadata of the rootfs
    rootFS.name = "rootfs";
    rootFS.mountName = "/";

    zero(&rootNode, sizeof(rootNode)); // clear the root node
    rootNode.filesystem = &rootFS;     // rootfs

    logInfo("vfs: rootfs mounted on %s", rootFS.mountName);
}

// add a node
struct vfs_node_t *vfsAdd(struct vfs_node_t node)
{
    uint64_t id = lastNode++;
    node.id = id;

    struct vfs_node_t *currentNode = &rootNode; // first node

    while (currentNode->next) // point to the last node
        currentNode = currentNode->next;

    currentNode->next = blkBlock(sizeof(struct vfs_node_t)); // allocate a new node
    currentNode = currentNode->next;                         // point to the new node

    memcpy64(currentNode, &node, sizeof(struct vfs_node_t) / sizeof(uint64_t)); // copy the node metadata

#ifdef K_VFS_DEBUG
    char buffer[512];
    vfsGetPath((uint64_t)&node, buffer);
    logDbg(LOG_SERIAL_ONLY, "vfs: adding node %s", buffer);
#endif

    return currentNode; // return its address
}

// add a drive node
void vfsAddDrive(vfs_drive_t drive)
{
    drives[lastDrive++] = drive;

#ifdef K_VFS_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vfs: registered drive %s on %s (%d MB)", drive.friendlyName, drive.interface, (uint64_t)drive.sectors * VFS_SECTOR / 1024 / 1024);
#endif

    // create a node for the drive
    struct vfs_node_t driveNode;
    zero(&driveNode, sizeof(driveNode));
    driveNode.filesystem = &rootFS;                                       // maybe we could create a devfs that can provide raw read/write functionality? (todo: implement this)
    sprintf(driveNode.path, "%s%s", drive.interface, drive.friendlyName); // copy the name

    vfsAdd(driveNode);

    for (int i = 0; i < 4; i++) // MBR only supported 4 partitions
    {
        if (!drive.partitions[i].startLBA)
            continue;

#ifdef K_VFS_DEBUG
        logDbg(LOG_SERIAL_ONLY, "vfs: partition %d starts at %d (%d MB)", i, drive.partitions[i].startLBA, (uint64_t)drive.partitions[i].sectors * VFS_SECTOR / 1024 / 1024);
#endif

#ifdef K_FAT

        // try to create a fat partition
        fat_bpb_t *bpb = pmmPage();
        drive.read(bpb, drive.partitions[i].startLBA, 1);

        if (!fatCreate(bpb, &drives[lastDrive - 1], i))
            pmmDeallocate(bpb);
#endif
    }
}

// get all available drives
vfs_drive_t *vfsGetDrives()
{
    return drives;
}

// remove a node
void vfsRemove(struct vfs_node_t *node)
{
    // todo: relink the nodes
    // todo: call the filesystem
    zero(node, sizeof(struct vfs_node_t)); // clear the node
}

// return the nodes
struct vfs_node_t *vfsNodes()
{
    return &rootNode;
}

// open a node with the name
uint64_t vfsOpen(const char *name)
{
    struct vfs_node_t *node = vfsGet(name);
    if (!ISVALID(node))
        return 0;

    // return the node if openning succeeds
    if (node->filesystem->open(node)) // the handler is guranteed to exist
        return (uint64_t)node;
    else
        return 0;
}

// close a node
void vfsClose(uint64_t fd)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node)) // check if the node is valid
        return;

    if (node->filesystem->close)       // check if the handler exists
        node->filesystem->close(node); // inform the filesystem that we closed the node
}

// read from a node in a buffer
void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node)) // check if the node is valid
        return;

    if (node->filesystem->read)                             // check if the handler exists
        node->filesystem->read(node, buffer, size, offset); // inform the filesystem that we want to read
}

// write to a node from a buffer
void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node)) // check if the node is valid
        return;

    if (node->filesystem->write)                             // check if the handler exists
        node->filesystem->write(node, buffer, size, offset); // inform the filesystem that we want to write
}

// get address of node with name
struct vfs_node_t *vfsGet(const char *name)
{
    if (!name || *name != '/' || !strlen(name)) // non-existent path
        return 0;

#ifdef K_VFS_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vfs: getting %s", name);
#endif

    struct vfs_node_t *currentNode = &rootNode;
    char fullPath[128 /* mount name */ + 128 /* path */];
    do
    {
        if (!currentNode->filesystem)
            goto next;

        vfsGetPath((uint64_t)currentNode, fullPath);

        if (strcmp(fullPath, name) != 0) // compare the paths
            goto next;

        if (!currentNode->filesystem->open) // check if open handler exists (it means it can be opened)
            goto next;

        return currentNode; // return the node
    next:
        currentNode = currentNode->next; // next node
    } while (currentNode);
    return 0; // return nothing
}

// return the size of a node
uint64_t vfsSize(uint64_t fd)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node)) // check if the node is valid
        return 0;

    return node->size; // return the size
}

// checks if file exists
bool vfsExists(const char *name)
{
    return vfsGet(name);
}

// gets full path of a node
void vfsGetPath(uint64_t fd, char *buffer)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node))
        return;

    sprintf(buffer, "%s%s", node->filesystem->mountName, node->path);
}

// check if mbr sector is valid
bool vfsCheckMBR(vfs_mbr_t *sector)
{
    return sector->signature == VFS_MBR_SIGNATURE;
}