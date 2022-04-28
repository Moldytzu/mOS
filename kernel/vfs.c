#include <vfs.h>

struct vfs_node nodes[0xFF]; // todo: replace with linked nodes
uint16_t lastNode = 0;

struct vfs_fs rootFS;

uint8_t rootOpen(struct vfs_node *fd)
{
    return 1; // status ok
}

void rootClose(struct vfs_node *fd)
{
    // do nothing
}

void vfsInit()
{
    memset64(nodes, 0, sizeof(nodes) / sizeof(uint64_t));    // clear the nodes
    memset64(&rootFS, 0, sizeof(rootFS) / sizeof(uint64_t)); // clear the root filesystem

    // metadata of the rootfs
    rootFS.name = "rootfs";
    rootFS.mountName = "/";
    rootFS.open = rootOpen;
    rootFS.close = rootClose;

    struct vfs_node node;                                           // the default node
    memset64(&node, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the node
    node.filesystem = &rootFS;                                      // rootfs
    memcpy(node.path, ".root", 5);                                  // copy the path ("/.root")
    vfsAdd(node);
}

void vfsAdd(struct vfs_node node)
{
    nodes[lastNode++] = node; // add the node
}

void vfsRemove(struct vfs_node *node)
{
    memset64(node, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the node
}

struct vfs_node *vfsNodes()
{
    return nodes;
}

uint64_t vfsOpen(const char *name)
{
    for (int i = 0; i < 0xFF; i++) // iterate over each node
    {
        if (nodes[i].filesystem)
        {
            if (memcmp(name, nodes[i].filesystem->mountName, strlen(nodes[i].filesystem->mountName)) == 0) // compare the mount name with the prefix
            {
                if (memcmp(name + strlen(nodes[i].filesystem->mountName), nodes[i].path, strlen(nodes[i].path)) == 0) // compare the path
                {
                    if(nodes[i].filesystem->open) // check if the handler exists
                        if (nodes[i].filesystem->open(&nodes[i])) // if the filesystem says that it is ok to open the file descriptor we return the address of the node
                            return (uint64_t)&nodes[i];
                }
            }
        }
    }
    return 0; // return nothing
}

void vfsClose(uint64_t fd)
{
    if (!fd) // don't handle empty/non-existent file descriptors
        return;

    struct vfs_node *node = (struct vfs_node *)fd;
    if(node->filesystem->close) // check if the handler exists
        node->filesystem->close(node); // inform the filesystem that we closed the node
}

void vfsRead(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
}

void vfsWrite(uint64_t fd, void *buffer, uint64_t size, uint64_t offset)
{
}