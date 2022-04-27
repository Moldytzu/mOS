#include <initrd.h>
#include <bootloader.h>

struct dsfs_fs *dsfs;

void initrdInit()
{
    dsfs = (struct dsfs_fs *)bootloaderGetModule("initrd.dsfs").begin;

    if (!dsfs)
    {
        bootloaderTermWrite("Failed to load the initrd from \"initrd.dsfs\".\nMake sure the file is in the root of the boot device.");
        hang();
    }
}

struct dsfs_entry *initrdGet(const char *name)
{
    struct dsfs_entry *entry = &dsfs->firstEntry; // point to the first entry

    for (uint32_t i = 0; i < dsfs->header.entries; i++)
    {
        if (memcmp(entry->name, name, strlen(name)) == 0) // compare the names
            return entry;

        entry = (struct dsfs_entry *)((uint64_t)entry + sizeof(struct dsfs_entry) + entry->size); // point to the next entry
    }

    return NULL; // return null if we don't find any entry with that name
}