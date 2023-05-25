#include <fs/fat.h>
#include <fs/vfs.h>
#include <misc/logger.h>
#include <mm/pmm.h>

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

// read handler
void fatFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
}

// open handler
uint8_t fatFSOpen(struct vfs_node_t *node)
{
    return 1;
}

bool fatDirValid(fat_dir_t dir)
{
    return dir.name[0] == 0 || dir.name[0] == 0xE5;
}

// map nodes in the vfs
void fatMap(struct vfs_node_t *root)
{
    size_t partitionIdx = CONTEXT(root->filesystem->context)->partition;
    vfs_partition_t partition = CONTEXT(root->filesystem->context)->drive->partitions[partitionIdx];
    fat_bpb_t *fs = CONTEXT(root->filesystem->context)->bpb;
    fat_context_t *context = CONTEXT(root->filesystem->context);

    if (fs->rootDirectoryEntries != 0 || FAT_CLUSTERS(fs) <= 65526) // not FAT32
        return;

    fat_dir_t *entries = pmmPage();

    context->drive->read(entries, partition.startLBA + FAT_ROOT_START(fs), 4096 / VFS_SECTOR);
    for (int i = 0; !fatDirValid(entries[i]); i++)
    {
        fat_dir_t entry = entries[i];

        if (entry.attributes.directory) // we don't support subdirectory traversal yet (todo: do that)
            continue;

        if (*(uint8_t *)&entry.attributes == 0xF) // long file name entry (todo: parse that)
            continue;

        int last = 0; // get last character that isn't a space
        for (; last < 8; last++)
            if (entry.name[last] == ' ')
                break;

        // parse 8.3 file name
        char name[12];
        zero(name, sizeof(name));
        memcpy(name, entry.name, last);             // copy the text before the extension
        name[last] = '.';                           // put the extension dot
        memcpy(name + last + 1, &entry.name[8], 3); // copy the extension

        for (int i = 0; i < 12; i++) // lower all the characters
            name[i] = tolower(name[i]);

        char *contents = pmmPage();
        
        size_t sector = partition.startLBA + (CLUSTER(entry) - 2) * fs->sectorsPerCluster + FAT_DATA_START(fs);
        context->drive->read(contents, sector, 1);
        
        printks("name: %s; attr: 0x%x; cluster: 0x%x; size: %d b; sector %d\n", name, entry.attributes, CLUSTER(entry), entry.size, sector);
        
        for (int j = 0; j < 16; j++)
            printks("%c", contents[j]);
        printks("\n");

        struct vfs_node_t node;             // create a node
        zero(&node, sizeof(node));          // zero it
        node.filesystem = root->filesystem; // set the filesystem
        node.size = entry.size;             // set the size
        memcpy(node.path, name, 12);        // copy the name
        vfsAdd(node);
    }
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
    const char *partStr = to_string(partition);

    // sprintf("%s%sp%d")
    memcpy(mountName, "/", 1);
    memcpy(mountName + 1, drive->interface, strlen(drive->interface));
    memcpy(mountName + strlen(drive->interface) + 1, drive->friendlyName, strlen(drive->friendlyName));
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + 1, "p", 1);
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + 2, partStr, strlen(partStr));
    memcpy(mountName + strlen(drive->interface) + strlen(drive->friendlyName) + strlen(partStr) + 2, "/", 1);

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