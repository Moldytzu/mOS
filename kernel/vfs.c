#include <vfs.h>

struct vfs_node nodes[0xFF]; // todo: replace with linked nodes
uint16_t lastNode = 0;

struct vfs_fs root;

uint8_t rootOpen(struct vfs_node *fd)
{
    return 1; // status ok
}

void vfsInit()
{
    memset64(nodes, 0, sizeof(nodes) / sizeof(uint64_t)); // clear the nodes
    memset64(&root, 0, sizeof(root) / sizeof(uint64_t));  // clear the root filesystem

    // metadata of the rootfs
    root.name = "rootfs";
    root.mountName = "/";
    root.open = rootOpen;

    struct vfs_node node;                                           // the default node
    memset64(&node, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the node
    node.filesystem = &root;                                        // rootfs
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
                    if(nodes[i].filesystem->open(&nodes[i])) // if the filesystem says that it is ok to open the file descriptor we return the address of the node
                        return (uint64_t)&nodes[i];
                }
            }
        }
    }
    return 0; // return nothing
}

void vfsClose(uint64_t fd)
{
}

void vfsRead(uint64_t fd, void *buffer, uint64_t size)
{
}

void vfsWrite(uint64_t fd, void *buffer, uint64_t size)
{
}