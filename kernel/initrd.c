#include <initrd.h>
#include <bootloader.h>
#include <vfs.h>

struct dsfs_fs *dsfs;
struct vfs_fs dsfsFS;

void initrdInit()
{
    dsfs = (struct dsfs_fs *)bootloaderGetModule("initrd.dsfs").begin;

    if (!dsfs)
    {
        bootloaderTermWrite("Failed to load the initrd from \"initrd.dsfs\".\nMake sure the file is in the root of the boot device.\n");
        hang();
    }

    if (dsfs->header.signature[0] != 'D' && dsfs->header.signature[1] != 'D')
    {
        bootloaderTermWrite("Failed to verify the signature of the initrd.\n");
        hang();
    }
}

uint8_t dsfsFSOpen(struct vfs_node *node)
{
    return 1; // status ok
}

void dsfsFSClose(struct vfs_node *node)
{
    printk("closed");
    // don't do anything
}

void initrdMount()
{
    dsfsFS.name = "dsfs";
    dsfsFS.mountName = "/init/";
    dsfsFS.open = dsfsFSOpen; // set the handlers
    dsfsFS.close = dsfsFSClose; 

    struct dsfs_entry *entry = &dsfs->firstEntry; // point to the first entry

    for (uint32_t i = 0; i < dsfs->header.entries; i++)
    {
        struct vfs_node node;
        memset64(&node, 0, sizeof(struct vfs_node) / sizeof(uint64_t)); // clear the node
        node.filesystem = &dsfsFS;                                      // set the ram filesystem
        memcpy64(node.path, entry->name, 56 / sizeof(uint64_t));        // copy the file path
        vfsAdd(node);

        entry = (struct dsfs_entry *)((uint64_t)entry + sizeof(struct dsfs_entry) + entry->size); // point to the next entry
    }
}

void *initrdGet(const char *name)
{
    struct dsfs_entry *entry = &dsfs->firstEntry; // point to the first entry

    for (uint32_t i = 0; i < dsfs->header.entries; i++)
    {
        if (memcmp(entry->name, name, strlen(name)) == 0)                 // compare the names
            return (void *)((uint64_t)entry + sizeof(struct dsfs_entry)); // point to the contents

        entry = (struct dsfs_entry *)((uint64_t)entry + sizeof(struct dsfs_entry) + entry->size); // point to the next entry
    }

    return NULL; // return null if we don't find any entry with that name
}