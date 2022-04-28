#include <vfs.h>

struct vfs_node nodes[0xFF]; // todo: replace with linked nodes
uint16_t lastNode = 0;

struct vfs_fs root;

void vfsInit()
{
    memset64(nodes, 0, sizeof(nodes) / sizeof(uint64_t)); // clear the nodes
    memset64(&root, 0, sizeof(root) / sizeof(uint64_t));  // clear the root filesystem

    // metadata of the rootfs
    root.name = "rootfs";
    root.mountName = "/";

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