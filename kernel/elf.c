#include <elf.h>
#include <elfabi.h>
#include <initrd.h>

bool elfLoad(const char *path)
{
    Elf64_Ehdr *elf = initrdGet(path); // get the elf header from the initrd

    if (!elf) // return if we didn't get the header
        return false;

    // check the compatibility
    if (elf->e_ident[EI_MAG0] != 0x7F || elf->e_ident[EI_MAG1] != 'E' || elf->e_ident[EI_MAG2] != 'L' || elf->e_ident[EI_MAG3] != 'F' ||
        elf->e_ident[EI_CLASS] != ELFCLASS64 || elf->e_ident[EI_DATA] != ELFDATA2LSB || elf->e_type != ET_EXEC || elf->e_machine != EM_X86_64 || elf->e_version != EV_CURRENT)
        return false;

    Elf64_Phdr *programHeaders = (Elf64_Phdr *)((uint64_t)elf + elf->e_phoff); // get the program headers from the offset

    return true;
}