#include <fs/vfs.h>
#include <fs/fat.h>
#include <mm/blk.h>
#include <mm/pmm.h>
#include <misc/logger.h>

#define NODE(x) ((vfs_node_t *)(x))

vfs_node_t *root;

// fixme: nothing is atomic here!

void vfsDumpLayer(vfs_node_t *node, uint16_t deep)
{
    // recursive function to dump an entire layer
    do
    {
        for (int i = 0; i < deep; i++)
            printks("-- ");

        printks(" %s = %s\n", node->name, node->flags & VFS_FLAG_DIRECTORY ? "dir" : "file");

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

    while (node)
    {
        if (strcmp(node->name, child) == 0)
            return node;

        node = NODE(node->next);
    }

    return NULL;
}

vfs_node_t *vfsCreateNodeAtPath(const char *path)
{
    printks("creating node: %s\n", path);

    // determine the depth of the path
    size_t depth = 0;
    for (size_t i = 0; path[i]; i++)
        if (path[i] == VFS_PATH_DELIMITER)
            depth++;

    printks("depth: %d\n", depth);

    path++; // skip the first delimiter

    // traverse the tree
    vfs_node_t *node = root;

    while (depth)
    {
        // find the next layer name
        // step 1: determine how long it's its name
        size_t layerNameSize = 0;
        while (path[layerNameSize] != 0 && path[layerNameSize] != VFS_PATH_DELIMITER)
            layerNameSize++;

        bool lastLayer = path[layerNameSize] == 0; // null termination of string
        bool isDirectory = !layerNameSize && path[layerNameSize] == '/';

        printks("layer name size: %d\n", layerNameSize);

        // step 2: copy its name in a buffer
        char *layerName = blkBlock(layerNameSize + 1);
        memcpy(layerName, path, layerNameSize);
        printks("in layer: %s\n", layerName);

        // step 3: create the node if we're the last layer
        if (isDirectory)
        {
            // fixme: mark it only and only if the node exists!!
            printks("%s is a directory now\n", node->name);
            node->flags |= VFS_FLAG_DIRECTORY;
            return node;
        }

        if (lastLayer)
        {
            if (vfsGetChildOf(node, layerName)) // already exists
            {
                printks("%s already exists in %s\n", layerName, node->name);

                node = vfsGetChildOf(node, layerName); // get that child
                blkDeallocate(layerName);              // deallocate
                return node;
            }

            vfs_node_t *newNode = blkBlock(sizeof(vfs_node_t));
            newNode->name = layerName;

            node->child = newNode;
            return newNode;
        }

        // step 4: if it isn't, move the node pointer to the child
        vfs_node_t *parent = node;
        node = vfsGetChildOf(node, layerName);

        if (!node) // it doesn't exist
        {
            // printks("achtung: %s doesn't exist\n", parent->name);
            //  todo: create here a path and use it to recursively create the node
        }

        // step 5: move the path forward
        path += layerNameSize;

        // step 6: clean up
        blkDeallocate(layerName);

        depth--;
    }

    return NULL;
}

void vfsInit()
{
    // create the root node
    root = blkBlock(sizeof(vfs_node_t));
    root->name = "/";
    root->flags |= VFS_FLAG_DIRECTORY;

    // // create a child called init
    // vfs_node_t *init = root->child = blkBlock(sizeof(vfs_node_t));
    // init->name = "init";

    // // make nodes in init
    // vfs_node_t *layer = init->child = blkBlock(sizeof(vfs_node_t));
    // layer->name = "abc";

    // layer = layer->next = blkBlock(sizeof(vfs_node_t));
    // layer->name = "def";

    // // create another child
    // vfs_node_t *ahci0p0 = init->next = blkBlock(sizeof(vfs_node_t));
    // ahci0p0->name = "ahci0p0";

    // // make some nodes in it
    // layer = ahci0p0->child = blkBlock(sizeof(vfs_node_t));
    // layer->name = "foo";

    // layer = layer->next = blkBlock(sizeof(vfs_node_t));
    // layer->name = "bar";

    // vfsCreateNodeAtPath("/etc/abc");

    vfsDumpSerial();

    while (1)
        ;
}