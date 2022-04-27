#include <elf.h>
#include <elfabi.h>
#include <initrd.h>

bool elfLoad(const char *path)
{
    Elf64_Ehdr *elf = initrdGet(path); // get the elf header from the initrd

    if (!elf) // return if we didn't get the header
        return false;

    // check the compatibility
    if (elf->e_ident[EI_CLASS] != ELFCLASS64 || elf->e_ident[EI_DATA] != ELFDATA2LSB || elf->e_type != ET_EXEC || elf->e_machine != EM_X86_64 || elf->e_version != EV_CURRENT)
        return false;

#ifdef K_ELF_DEBUG    
    printks("elf: found %s at 0x%p\n\r",path,elf);
#endif

    Elf64_Phdr *phdr = (Elf64_Phdr *)((uint64_t)elf + elf->e_phoff); // get the program headers from the offset

    for(int i = 0; i < elf->e_phnum; i++, phdr++) // iterate over every program header
    {
        if(phdr->p_type == PT_LOAD) // section to be loaded
        {
#ifdef K_ELF_DEBUG
    printks("elf: phdr at virtual 0x%p (physical 0x%p) with size %d bytes\n\r",phdr->p_vaddr,phdr->p_paddr,phdr->p_memsz);
#endif
        }
    }

    return true;
}