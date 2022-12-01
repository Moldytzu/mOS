#include <drv/initrd.h>
#include <fw/bootloader.h>
#include <fs/vfs.h>

dsfs_fs_t *dsfs;
vfs_fs_t dsfsFS;

// initialize the initrd
void initrdInit()
{
    dsfs = (dsfs_fs_t *)bootloaderGetModule("initrd.dsfs")->address; // get the begining of the file

    if (!dsfs)
    {
        bootloaderWrite("Failed to load the initrd from \"initrd.dsfs\".\nMake sure the file is in the root of the boot device.\n");
        hang();
    }

    if (dsfs->header.signature[0] != 'D' && dsfs->header.signature[1] != 'D')
    {
        bootloaderWrite("Failed to verify the signature of the initrd.\n");
        hang();
    }
}

// open handler
uint8_t dsfsFSOpen(struct vfs_node_t *node)
{
    return initrdGet(node->path) > 0; // return 1 if it is valid
}

// close handler
void dsfsFSClose(struct vfs_node_t *node)
{
    // don't do anything
}

// read handler
void dsfsFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
    memset8(buffer, 0, size); // clear the buffer
    void *entry = initrdGet(node->path);
    if (entry) // copy only if the entry is present
    {
        // limit the size
        if (size > ((dsfs_entry_t *)((uint64_t)entry - sizeof(dsfs_entry_t)))->size)
            size = ((dsfs_entry_t *)((uint64_t)entry - sizeof(dsfs_entry_t)))->size;
        memcpy(buffer, entry + offset, size); // do the actual copy
    }
}

// write handler
void dsfsFSWrite(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
    // do nothing
}

// mount the filesystem
void initrdMount()
{
    dsfsFS.name = "dsfs";
    dsfsFS.mountName = "/init/";
    dsfsFS.open = dsfsFSOpen; // set the handlers
    dsfsFS.close = dsfsFSClose;
    dsfsFS.read = dsfsFSRead;
    dsfsFS.write = dsfsFSWrite;

    dsfs_entry_t *entry = &dsfs->firstEntry; // point to the first entry

    struct vfs_node_t node; // root node
    zero(&node, sizeof(struct vfs_node_t));
    node.filesystem = &dsfsFS; // set the ram filesystem
    vfsAdd(node);

    for (uint32_t i = 0; i < dsfs->header.entries; i++)
    {
        zero(&node, sizeof(struct vfs_node_t));
        node.filesystem = &dsfsFS;                               // set the ram filesystem
        node.size = entry->size;                                 // set the size
        memcpy64(node.path, entry->name, 56 / sizeof(uint64_t)); // copy the file path
        vfsAdd(node);

        entry = (dsfs_entry_t *)((uint64_t)entry + sizeof(dsfs_entry_t) + entry->size); // point to the next entry
    }
}

// get an entry from the filesystem
void *initrdGet(const char *name)
{
    dsfs_entry_t *entry = &dsfs->firstEntry; // point to the first entry

    for (uint32_t i = 0; i < dsfs->header.entries; i++)
    {
        if (memcmp(entry->name, name, strlen(name)) == 0)            // compare the names
            return (void *)((uint64_t)entry + sizeof(dsfs_entry_t)); // point to the contents

        entry = (dsfs_entry_t *)((uint64_t)entry + sizeof(dsfs_entry_t) + entry->size); // point to the next entry
    }

    return NULL; // return null if we don't find any entry with that name
}