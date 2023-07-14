#include <fs/initrd.h>
#include <fw/bootloader.h>
#include <fs/vfs.h>
#include <misc/logger.h>

#define FIRST_ENTRY (dsfs_entry_t *)((uint64_t)dsfs + sizeof(dsfs_header_t))
#define CONTENTS_OF(x) (void *)((uint64_t)x + sizeof(dsfs_entry_t))
#define NEXT_ENTRY_OF(x) ((dsfs_entry_t *)((uint64_t)x + sizeof(dsfs_entry_t) + x->size))

dsfs_header_t *dsfs;
vfs_fs_t dsfsFS;

// initialize the initrd
void initrdInit()
{
    dsfs = (dsfs_header_t *)bootloaderGetModule("initrd.dsfs")->address; // get the begining of the file

    if (!dsfs)
    {
        logError("dsfs: failed to load the initrd from \"initrd.dsfs\".\nmake sure the file is in the root of the boot device\n");
        hang();
    }

    if (dsfs->signature[0] != 'D' && dsfs->signature[1] != 'D')
    {
        logError("dsfs: failed to verify the signature\n");
        hang();
    }
}

// open handler
uint8_t dsfsFSOpen(struct vfs_node_t *node)
{
    return 1; // always a success
}

// read handler
void dsfsFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
    dsfs_entry_t *entry = initrdGet(node->path);

    if (!entry) // fail
        return;

    // limit the size
    if (size > entry->size)
        size = entry->size;

    memcpy(buffer, CONTENTS_OF(entry) + offset, size); // do the actual copy
}

// mount the filesystem
void initrdMount()
{
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

    for (uint32_t i = 0; i < dsfs->entries; i++) // iterate over all entries of the filesystem
    {
        node.size = entry->size;                                 // set the size
        memcpy64(node.path, entry->name, 56 / sizeof(uint64_t)); // copy the file path
        vfsAdd(node);                                            // add the node

        entry = NEXT_ENTRY_OF(entry); // point to the next entry
    }
}

// get an entry from the filesystem
dsfs_entry_t *initrdGet(const char *name)
{
    dsfs_entry_t *entry = FIRST_ENTRY; // point to the first entry

    for (uint32_t i = 0; i < dsfs->entries; i++) // iterate over all entries of the filesystem
    {
        if (memcmp(entry->name, name, strlen(name)) == 0) // compare the names
            return entry;                                 // return the address

        entry = NEXT_ENTRY_OF(entry); // point to the next entry
    }

    return NULL; // return null if we don't find any entry with that name
}