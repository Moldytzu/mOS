#include <fs/fat.h>
#include <fs/vfs.h>
#include <misc/logger.h>
#include <mm/pmm.h>

// todo: before adding any other feature to this, clean this mess

#ifdef K_FAT

#define CONTEXT(x) ((fat_context_t *)(x))
#define CLUSTER(x) (x.clusterHigh << 16 | x.clusterLow)

#define FAT_START(fs) (fs->reservedSectors)
#define FAT_SECTORS(fs) (fs->sectorsPerFat * fs->fats)
#define FAT_ROOT_START(fs) (FAT_START(fs) + FAT_SECTORS(fs))
#define FAT_ROOT_SECTORS(fs) ((32 * fs->rootDirectoryEntries + fs->bytesPerSector - 1) / fs->bytesPerSector)
#define FAT_DATA_START(fs) (FAT_ROOT_START(fs) + FAT_ROOT_SECTORS(fs))
#define FAT_DATA_SECTORS(fs) (fs->totalSectors32 - FAT_DATA_START(fs))
#define FAT_CLUSTERS(fs) (FAT_DATA_SECTORS(fs) / fs->sectorsPerCluster)

bool fatIsValid(fat_bpb_t *bpb)
{
    return bpb->bootSignature == VFS_MBR_SIGNATURE;
}

bool fatDirLast(fat_dir_t dir)
{
    return dir.name[0] == 0;
}

bool fatDirValid(fat_dir_t dir)
{
    return !fatDirLast(dir) || (dir.name[0] != 0x05 && dir.name[0] != 0xE5);
}

void fatParseSFN(char *name, fat_dir_t *entry)
{
    int last = 0; // get last character that isn't a space
    for (; last < 8; last++)
        if (entry->name[last] == ' ')
            break;

    // parse 8.3 file name
    memcpy(name, entry->name, last);             // copy the text before the extension
    name[last] = '.';                            // put the extension dot
    memcpy(name + last + 1, &entry->name[8], 3); // copy the extension

#ifdef K_FAT_LOWER_SFN
    for (int i = 0; i < 12; i++) // lower all the characters
        name[i] = tolower(name[i]);
#endif
}

size_t fatParseLFN(char *name, fat_lfn_t *entry)
{
    size_t index = 0;
    for (int i = 0; i < 5; i++)
        name[index++] = entry->name1[i] & 0xFF;
    for (int i = 0; i < 6; i++)
        name[index++] = entry->name2[i] & 0xFF;
    for (int i = 0; i < 2; i++)
        name[index++] = entry->name3[i] & 0xFF;

    return entry->order & ~0x40; // clean up to get the order
}

size_t fatRootEntries_impl(struct vfs_node_t *root, size_t offset)
{
    fat_context_t *context = CONTEXT(root->filesystem->context);
    size_t partitionIdx = context->partition;
    vfs_partition_t partition = context->drive->partitions[partitionIdx];
    fat_bpb_t *fs = context->bpb;

    fat_dir_t *entries = pmmPage(); // can hold 128 entries

    char lname[256];
    context->drive->read(entries, partition.startLBA + FAT_ROOT_START(fs) + (PMM_PAGE / VFS_SECTOR) * offset, PMM_PAGE / VFS_SECTOR);

    if (fatDirLast(entries[127])) // we don't have more entries
    {
        size_t s = 0;
        for (; s < 128 && !fatDirLast(entries[s]); s++)
            ; // determine entries in this block
        s += offset * 128;

        pmmDeallocate(entries);
        return s;
    }

    pmmDeallocate(entries);
    return fatRootEntries_impl(root, offset + 1); // recursively calculate for next block
}

size_t fatRootEntries(struct vfs_node_t *root)
{
    return fatRootEntries_impl(root, 0);
}

fat_dir_t fatGetEntry(struct vfs_node_t *node)
{
    fat_context_t *context = CONTEXT(node->filesystem->context);
    size_t partitionIdx = context->partition;
    vfs_partition_t partition = context->drive->partitions[partitionIdx];
    fat_bpb_t *fs = context->bpb;

    size_t maxEntries = fatRootEntries(node); // todo: maybe we could cache this in the context
    size_t pages = maxEntries / 128 + 1;
    fat_dir_t *entries = pmmPages(pages);

    char lname[256];
    context->drive->read(entries, partition.startLBA + FAT_ROOT_START(fs), pages * 4096 / VFS_SECTOR);
    for (int i = 0; i < maxEntries; i++)
    {
        fat_dir_t entry = entries[i];

        if (!fatDirValid(entry)) // invalid entry
            continue;

        if (entry.attributes.directory) // we don't support subdirectory traversal yet (todo: do that)
            continue;

        if (*(uint8_t *)&entry.attributes == 0xF) // long file name entry
        {
            fat_lfn_t lfn = *(fat_lfn_t *)&entry;

            zero(lname, sizeof(lname));

            size_t order = lfn.order & ~0x40; // strip out the bit that indicates the owner

            for (int k = 0; k < order; k++)
            {
                fat_lfn_t l = *(fat_lfn_t *)&entries[i + k];
                fatParseLFN(lname + 13 * (order - 1 - k), &l); // parse then reverse its location because lfns are sequencially ordered (from x to 1 where x is a natural number)
            }

            i += order - 1;

            continue;
        }

        // parse 8.3 file name
        char name[12];
        zero(name, sizeof(name));
        fatParseSFN(name, &entry);

        logDbg(LOG_SERIAL_ONLY, "fat: found entry %s with size %d b", lname, entry.size);

        if (strcmp(node->path, lname) != 0 && strcmp(node->path, name) != 0) // compare the names
            continue;

        pmmDeallocatePages(entries, pages);
        return entry;
    }

    pmmDeallocatePages(entries, pages);
    fat_dir_t d;
    zero(&d, sizeof(d));
    return d;
}

// read handler
void fatFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
    fat_dir_t entry = fatGetEntry(node);
    if (!fatDirValid(entry))
        return;

    fat_context_t *context = CONTEXT(node->filesystem->context);
    size_t partitionIdx = context->partition;
    vfs_partition_t partition = context->drive->partitions[partitionIdx];
    fat_bpb_t *fs = context->bpb;

    size_t pages = size / PMM_PAGE + 1;
    size_t sectors = size / VFS_SECTOR + 1;
    size_t sector = partition.startLBA + (CLUSTER(entry) - 2) * fs->sectorsPerCluster + FAT_DATA_START(fs);

    void *tmp = pmmPages(pages);                // allocate a temporary buffer
    context->drive->read(tmp, sector, sectors); // call the drive
    memcpy(buffer, tmp + offset, size);         // copy the sector
    pmmDeallocatePages(tmp, pages);             // deallocate
}

// open handler
uint8_t fatFSOpen(struct vfs_node_t *node)
{
    return 1;
}

void mapSubdirectory(struct vfs_node_t *root, fat_dir_t *entries, size_t i)
{
    fat_dir_t entry = entries[i];

    struct vfs_node_t node;             // create a node
    zero(&node, sizeof(node));          // zero it
    node.filesystem = root->filesystem; // set the filesystem

    size_t nameSize = 0;
    for (; entry.name[nameSize] != ' '; nameSize++)
        ; // determine the size of the name

    memcpy(node.path, entry.name, nameSize); // copy the name
    node.path[nameSize] = '/';

    char path[64];
    vfsGetPath((uint64_t)vfsAdd(node), path); // create the node
    logDbg(LOG_SERIAL_ONLY, "fat: found directory %s, mounted at %s", entry.name, path);
}

// map nodes in the vfs
void fatMap(struct vfs_node_t *root)
{
    fat_context_t *context = CONTEXT(root->filesystem->context);
    size_t partitionIdx = context->partition;
    vfs_partition_t partition = context->drive->partitions[partitionIdx];
    fat_bpb_t *fs = context->bpb;

    if (fs->rootDirectoryEntries != 0 || FAT_CLUSTERS(fs) <= 65526) // not FAT32
        return;

    size_t maxEntries = fatRootEntries(root);
    size_t pages = maxEntries / 128 + 1;
    fat_dir_t *entries = pmmPages(pages);

    char lname[256];
    context->drive->read(entries, partition.startLBA + FAT_ROOT_START(fs), pages * 4096 / VFS_SECTOR);
    for (int i = 0; i < maxEntries; i++)
    {
        fat_dir_t entry = entries[i];

        if (!fatDirValid(entry)) // invalid entry
            continue;

        if (entry.attributes.directory) // we don't support subdirectory traversal yet (todo: do that)
        {
            mapSubdirectory(root, entries, i);
            continue;
        }

        if (*(uint8_t *)&entry.attributes == 0xF) // long file name entry
        {
            fat_lfn_t lfn = *(fat_lfn_t *)&entry;

            zero(lname, sizeof(lname));

            size_t order = lfn.order & ~0x40; // strip out the bit that indicates the owner

            for (int k = 0; k < order; k++)
            {
                fat_lfn_t l = *(fat_lfn_t *)&entries[i + k];
                fatParseLFN(lname + 13 * (order - 1 - k), &l); // parse then reverse its location because lfns are sequencially ordered (from x to 1 where x is a natural number)
            }

            i += order - 1;

            continue;
        }

        // parse 8.3 file name
        char name[13];
        zero(name, sizeof(name));
        fatParseSFN(name, &entry);

        logDbg(LOG_SERIAL_ONLY, "fat: found entry %s with size %d b", lname, entry.size);

        struct vfs_node_t node;             // create a node
        zero(&node, sizeof(node));          // zero it
        node.filesystem = root->filesystem; // set the filesystem
        node.size = entry.size;             // set the size

        if (strlen(lname))
            memcpy(node.path, lname, min(strlen(lname), 127)); // copy the long name
        else
            memcpy(node.path, name, 12); // copy the short name

        vfsAdd(node);
        zero(lname, sizeof(lname));
    }

    pmmDeallocatePages(entries, pages);
}

// create a new fat context
bool fatCreate(fat_bpb_t *bpb, vfs_drive_t *drive, size_t partition)
{
    if (!fatIsValid(bpb))
        return false;

    // create a vfs-compatible context
    vfs_fs_t *filesystem = pmmPage();
    filesystem->context = pmmPage();

    // create an internal context
    CONTEXT(filesystem->context)->bpb = bpb;
    CONTEXT(filesystem->context)->drive = drive;
    CONTEXT(filesystem->context)->partition = partition;

    // generate a mount name
    char *mountName = pmmPage();
    sprintf(mountName, "/%s%sp%d/", drive->interface, drive->friendlyName, partition);

    logDbg(LOG_SERIAL_ONLY, "fat: mounting filesystem as %s", mountName);

    filesystem->name = "fat";
    filesystem->mountName = mountName;

    // set handlers
    filesystem->read = fatFSRead;
    filesystem->open = fatFSOpen;

    // create root node
    struct vfs_node_t rootNode;
    zero(&rootNode, sizeof(rootNode));
    rootNode.filesystem = filesystem;

    fatMap(vfsAdd(rootNode));

    return true;
}
#endif