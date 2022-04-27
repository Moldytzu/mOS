#include <elf.h>
#include <elfabi.h>
#include <initrd.h>

bool elfLoad(const char *path)
{
    Elf64_Ehdr *elf = initrdGet(path); // get the elf header from the initrd

    if(!elf) // return if we didn't get the header
        return false;

    return true;
}