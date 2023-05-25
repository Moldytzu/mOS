#include <fs/fat.h>
#include <fs/vfs.h>
#include <misc/logger.h>
#include <mm/pmm.h>

#define CONTEXT(x) ((fat_context_t *)(x))

bool fatIsValid(fat_bpb_t *bpb)
{
    return bpb->bootSignature == VFS_MBR_SIGNATURE;
}

void fatFSRead(struct vfs_node_t *node, void *buffer, uint64_t size, uint64_t offset)
{
}

bool isFree(fat_dir_t dir)
{
    return dir.name[0] == 0 || dir.name[0] == 0xE5;
}

// map nodes in the vfs
void fatMap(struct vfs_node_t *root)
{
    size_t partitionIdx = CONTEXT(root->filesystem->context)->partition;
    vfs_partition_t parition = CONTEXT(root->filesystem->context)->drive->partitions[partitionIdx];
    fat_bpb_t *fs = CONTEXT(root->filesystem->context)->bpb;

    uint32_t fatStartSector = fs->reservedSectors;
    uint32_t fatSectors = fs->sectorsPerFat * fs->fats;

    uint32_t rootDirectoryStartSector = fatStartSector + fatSectors;
    uint32_t rootDirectorySectors = (32 * fs->rootDirectoryEntries + fs->bytesPerSector - 1) / fs->bytesPerSector;

    uint32_t dataStartSector = rootDirectoryStartSector + rootDirectorySectors;
    uint32_t dataSectors = fs->totalSectors32 - dataStartSector;

    uint32_t clusters = dataSectors / fs->sectorsPerCluster;

    if (fs->rootDirectoryEntries != 0 || clusters <= 65526) // not FAT32
        return;

    fat_dir_t *rootDirectory = pmmPage();

    CONTEXT(root->filesystem->context)->drive->read(rootDirectory, parition.startLBA + rootDirectoryStartSector, 1);
    for (int i = 0; !isFree(rootDirectory[i]); i++)
    {
        if (rootDirectory[i].attributes.directory) // we don't support subdirectory traversal yet (todo: do that)
            continue;

        if (*(uint8_t *)&rootDirectory[i].attributes == 0xF) // long file name entry (todo: parse that)
            continue;

        uint32_t cluster = rootDirectory[i].clusterHigh << 16 | rootDirectory[i].clusterLow;

        int last = 0; // get last character that isn't a space
        for (; last < 8; last++)
            if (rootDirectory[i].name[last] == ' ')
                break;

        char name[12];
        zero(name, sizeof(name));
        memcpy(name, rootDirectory[i].name, last);
        name[last] = '.';
        memcpy(name + last + 1, &rootDirectory[i].name[8], 3);

        for(int i = 0; i < 12; i++)
            name[i] = tolower(name[i]);

        printks("name: %s", name);

        printks("; attr: 0x%x; cluster: 0x%x; size: %d b\n", rootDirectory[i].attributes, cluster, rootDirectory[i].size);

        struct vfs_node_t node;
        zero(&node, sizeof(node));
        node.filesystem = root->filesystem;
        memcpy(node.path, name, 12);
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

    // create root node
    struct vfs_node_t rootNode;
    zero(&rootNode, sizeof(rootNode));
    rootNode.filesystem = filesystem;

    fatMap(vfsAdd(rootNode));

    return true;
}