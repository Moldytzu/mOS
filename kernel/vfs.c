#include <vfs.h>
#include <heap.h>

struct vfs_node rootNode; // root of the linked list
uint64_t lastNode = 0;

struct vfs_fs rootFS;

// initialize the subsystem
void vfsInit()
{
    memset64(&rootFS, 0, sizeof(rootFS) / sizeof(uint64_t)); // clear the root filesystem

    // metadata of the rootfs
    rootFS.name = "rootfs";
    rootFS.mountName = "/";

    memset64(&rootNode, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the root node
    rootNode.filesystem = &rootFS;                                      // rootfs
}

// add a node
void vfsAdd(struct vfs_node node)
{
    uint64_t id = lastNode++;
    node.id = id;

    struct vfs_node *currentNode = &rootNode; // first node

    if (currentNode->filesystem) // check if the root node is valid
    {
        while (currentNode->next) // get last node
            currentNode = currentNode->next;

        if (currentNode->filesystem)
        {
            currentNode->next = malloc(sizeof(struct vfs_node)); // allocate next node if the current node is valid
            currentNode = currentNode->next;                     // set current node to the newly allocated node
        }
    }
    memcpy64(currentNode, &node, sizeof(struct vfs_node) / sizeof(uint64_t)); // copy the node information
}

// remove a node
void vfsRemove(struct vfs_node *node)
{
    memset64(node, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the node
}

// return the nodes
struct vfs_node *vfsNodes()
{
    return &rootNode;
}

// open a node with the name
uint64_t vfsOpen(const char *name)
{
    struct vfs_node *currentNode = &rootNode;
    const char tmp[128 /* mount name */ + 128 /* path */];
    do
    {
        if (!currentNode->filesystem)
            goto next;

        // memory functions equivalent of sprintf("%s%s"...);
        memset64((void *)tmp, 0, sizeof(tmp) / sizeof(uint64_t));
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
    if (!fd) // don't handle empty/non-existent file descriptors
        return;

    struct vfs_node *node = (struct vfs_node *)fd;
    if (node->filesystem->close)       // check if the handler exists
        node->filesystem->close(node); // inform the filesystem that we closed the node
}

// read from a node in a buffer
void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
    if (!fd) // don't handle empty/non-existent file descriptors
        return;

    struct vfs_node *node = (struct vfs_node *)fd;
    if (node->filesystem->read)                             // check if the handler exists
        node->filesystem->read(node, buffer, size, offset); // inform the filesystem that we want to read
}

// write to a node from a buffer
void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
    if (!fd) // don't handle empty/non-existent file descriptors
        return;

    struct vfs_node *node = (struct vfs_node *)fd;
    if (node->filesystem->write)                             // check if the handler exists
        node->filesystem->write(node, buffer, size, offset); // inform the filesystem that we want to write
}

// return the size of a node
uint64_t vfsSize(uint64_t fd)
{
    if (!fd) // don't handle empty/non-existent file descriptors
        return 0;

    struct vfs_node *node = (struct vfs_node *)fd;
    return node->size; // return the size
}