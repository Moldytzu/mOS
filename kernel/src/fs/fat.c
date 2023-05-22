#include <fs/fat.h>
#include <fs/vfs.h>
#include <misc/logger.h>

bool fatIsValid(fat_bpb_t *bpb)
{
    return bpb->bootSignature == VFS_MBR_SIGNATURE;
}