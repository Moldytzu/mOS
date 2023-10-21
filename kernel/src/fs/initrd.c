#include <fs/initrd.h>
#include <fw/bootloader.h>
#include <fs/vfs.h>
#include <misc/logger.h>

// reference implementation of DSFS 1.0b specification

#define FIRST_ENTRY (dsfs_entry_t *)((uint64_t)dsfs + sizeof(dsfs_header_t))
#define CONTENTS_OF(x) (void *)((uint64_t)x + sizeof(dsfs_entry_t))
#define NEXT_ENTRY_OF(x) ((dsfs_entry_t *)((uint64_t)x + sizeof(dsfs_entry_t) + x->size))
#define IS_LAST(x) (x->size == 0 && entry->name[0] == 0)

dsfs_header_t *dsfs;
// vfs_fs_t dsfsFS;

// initialize the initrd
void initrdInit()
{
    struct limine_file *file = bootloaderGetModule("initrd.dsfs");
    if (!file)
    {
        dsfs = NULL;
        logWarn("dsfs: failed to read filesystem from \"initrd.dsfs\"");
        return;
    }

    dsfs = (dsfs_header_t *)file->address; // get the begining of the file

    if (dsfs->signature[0] != 'D' && dsfs->signature[1] != 'D')
    {
        logError("dsfs: failed to verify the signature\n");
        hang();
    }
}

/*
// mount the filesystem
void initrdMount()
{
    if (!dsfs)
        return;

    // set metadata for filesystem
    dsfsFS.name = "dsfs";
    dsfsFS.mountName = "/init/";
    dsfsFS.open = dsfsFSOpen;
    dsfsFS.read = dsfsFSRead;

    dsfs_entry_t *entry = FIRST_ENTRY; // point to the first entry

    struct vfs_node_t node;                 // root node
    zero(&node, sizeof(struct vfs_node_t)); // clear it
    node.filesystem = &dsfsFS;              // set the filesystem
    vfsAdd(node);                           // create the mount

    while (!IS_LAST(entry)) // while this isn't the end of the chain
    {
        node.size = entry->size;                                 // set the size
        memcpy64(node.path, entry->name, 56 / sizeof(uint64_t)); // copy the file path
        vfsAdd(node);                                            // add the node

        entry = NEXT_ENTRY_OF(entry); // point to the next entry
    }
}
*/

// get an entry from the filesystem
dsfs_entry_t *initrdGet(const char *name)
{
    dsfs_entry_t *entry = FIRST_ENTRY; // point to the first entry

    while (!IS_LAST(entry)) // while this isn't the end of the chain
    {
        if (memcmp(entry->name, name, strlen(name)) == 0) // compare the names
            return entry;                                 // return the address

        entry = NEXT_ENTRY_OF(entry); // point to the next entry
    }

    return NULL; // return null if we don't find any entry with that name
}