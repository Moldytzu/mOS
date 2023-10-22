#include <fs/vfs.h>
#include <fs/fat.h>
#include <mm/blk.h>
#include <mm/pmm.h>
#include <misc/logger.h>

#define NODE(x) ((vfs_node_t *)(x))

vfs_node_t *root;

void vfsDumpLayer(vfs_node_t *node, uint16_t deep)
{
    // recursive function to dump an entire layer
    do
    {
        for (int i = 0; i < deep; i++)
            printks("-- ");

        printks(" %s\n", node->name);

        if (node->child) // dump this layer
            vfsDumpLayer(node->child, deep + 1);

        node = NODE(node->next);
    } while (node);
}

void vfsDumpSerial()
{
    // dump the whole vfs tree in serial console
    vfs_node_t *node = root;
    do
    {
        vfsDumpLayer(node, 0);

        node = NODE(node->next);
    } while (node);
}

vfs_node_t *vfsGetChildOf(vfs_node_t *node, const char *child)
{
    // scan the node for the requested child
    node = node->child; // point to the child list

    do
    {
        if (strcmp(node->name, child) == 0)
            return node;

        node = NODE(node->next);
    } while (node);

    return NULL;
}

vfs_node_t *vfsCreateNode(const char *path)
{
    // traverse the tree
    vfs_node_t *root;

    return NULL;
}

void vfsInit()
{
    // create the root node
    root = blkBlock(sizeof(vfs_node_t));
    root->name = "/";

    // create a child called init
    vfs_node_t *init = root->child = blkBlock(sizeof(vfs_node_t));
    init->name = "init";

    // make nodes in init
    vfs_node_t *layer = init->child = blkBlock(sizeof(vfs_node_t));
    layer->name = "abc";

    layer = layer->next = blkBlock(sizeof(vfs_node_t));
    layer->name = "def";

    // create another child
    vfs_node_t *ahci0p0 = init->next = blkBlock(sizeof(vfs_node_t));
    ahci0p0->name = "ahci0p0";

    // make some nodes in it
    layer = ahci0p0->child = blkBlock(sizeof(vfs_node_t));
    layer->name = "foo";

    layer = layer->next = blkBlock(sizeof(vfs_node_t));
    layer->name = "bar";

    vfsDumpSerial();

    printks("/init %s\n", vfsGetChildOf(root, "init")->name);

    while (1)
        ;
}