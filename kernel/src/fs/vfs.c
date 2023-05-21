#include <fs/vfs.h>
#include <mm/blk.h>
#include <misc/logger.h>

#define ISVALID(node) (node && node->filesystem)

struct vfs_node_t rootNode; // root of the linked list
uint64_t lastNode = 0;

vfs_fs_t rootFS;

// initialize the subsystem
void vfsInit()
{
    zero(&rootFS, sizeof(rootFS)); // clear the root filesystem

    // metadata of the rootfs
    rootFS.name = "rootfs";
    rootFS.mountName = "/";

    zero(&rootNode, sizeof(rootNode)); // clear the root node
    rootNode.filesystem = &rootFS;     // rootfs

    logInfo("vfs: rootfs mounted on %s", rootFS.mountName);
}

// add a node
void vfsAdd(struct vfs_node_t node)
{
    uint64_t id = lastNode++;
    node.id = id;

    struct vfs_node_t *currentNode = &rootNode; // first node

    if (currentNode->filesystem) // check if the root node is valid
    {
        while (currentNode->next) // get last node
            currentNode = currentNode->next;

        if (currentNode->filesystem)
        {
            currentNode->next = blkBlock(sizeof(struct vfs_node_t)); // allocate next node if the current node is valid
            currentNode = currentNode->next;                         // set current node to the newly allocated node
        }
    }
    memcpy64(currentNode, &node, sizeof(struct vfs_node_t) / sizeof(uint64_t)); // copy the node information

#ifdef K_VFS_DEBUG
    char buffer[512];
    zero(buffer, sizeof(buffer));
    vfsGetPath((uint64_t)&node, buffer);
    logDbg(LOG_SERIAL_ONLY, "vfs: adding node %s", buffer);
#endif
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
    if (!name || *name != '/' || !strlen(name)) // non-existent path
        return 0;

#ifdef K_VFS_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vfs: opening %s", name);
#endif

    struct vfs_node_t *currentNode = &rootNode;
    const char tmp[128 /* mount name */ + 128 /* path */];
    do
    {
        if (!currentNode->filesystem)
            goto next;

        // memory functions equivalent of sprintf("%s%s"...);
        zero((void *)tmp, sizeof(tmp));
        memcpy((void *)tmp, currentNode->filesystem->mountName, strlen(currentNode->filesystem->mountName));
        memcpy((void *)(tmp + strlen(currentNode->filesystem->mountName)), currentNode->path, strlen(currentNode->path));

        if (strcmp(tmp, name) != 0) // compare the temp and the name
            goto next;

        if (!currentNode->filesystem->open) // check if the handler exists
            goto next;

        if (currentNode->filesystem->open(currentNode)) // if the filesystem says that it is ok to open the file descriptor we return the address of the node
            return (uint64_t)currentNode;

    next:
        currentNode = currentNode->next; // next node
    } while (currentNode);
    return 0; // return nothing
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
    uint64_t fd = vfsOpen(name);

    if (fd)
        vfsClose(fd);

    return fd > 0;
}

// gets full path of a node
void vfsGetPath(uint64_t fd, void *buffer)
{
    struct vfs_node_t *node = (struct vfs_node_t *)fd;
    if (!ISVALID(node))
        return;

    memcpy((void *)buffer, node->filesystem->mountName, strlen(node->filesystem->mountName));
    memcpy((void *)(buffer + strlen(node->filesystem->mountName)), node->path, strlen(node->path));
}