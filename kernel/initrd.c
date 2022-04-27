#include <initrd.h>
#include <bootloader.h>

struct dsfs_header *dsfs;

void initrdInit()
{
    dsfs = (struct dsfs_header *)bootloaderGetModule("initrd.dsfs").begin;
    
    if(!dsfs)
    {
        bootloaderTermWrite("Failed to load the initrd from \"initrd.dsfs\".\nMake sure the file is in the root of the boot device.");
        hang();
    }
}