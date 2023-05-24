#include <elf/elf.h>
#include <elf/elfabi.h>
#include <fs/vfs.h>
#include <mm/pmm.h>
#include <mm/blk.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

// load an elf
sched_task_t *elfLoad(const char *path, int argc, char **argv, bool driver)
{
    uint64_t fd = vfsOpen(path);                    // open the file
    uint64_t fdSize = vfsSize(fd);                  // get the size
    Elf64_Ehdr *elf = blkBlock(sizeof(Elf64_Ehdr)); // allocate the elf header

    vfsRead(fd, elf, sizeof(Elf64_Ehdr), 0); // read the header

    // check compatibility
    if (elf->e_ident[EI_CLASS] != ELFCLASS64 || elf->e_ident[EI_DATA] != ELFDATA2LSB || elf->e_type != ET_EXEC || elf->e_machine != EM_X86_64 || elf->e_version != EV_CURRENT)
    {
        // clean up
        blkDeallocate(elf);
        vfsClose(fd);
        return false;
    }

#ifdef K_ELF_DEBUG
    logDbg(LOG_SERIAL_ONLY, "elf: found %s at 0x%p with the entry offset at 0x%p", path, elf, elf->e_entry - TASK_BASE_ADDRESS);
#endif

    if (fdSize % 4096 != 0) // make sure the executable size is divisible by a page
        fdSize += fdSize - (fdSize % 4096);

    void *buffer = pmmPages(fdSize / VMM_PAGE); // allocate the buffer for the sections

    Elf64_Phdr *phdr = blkBlock(elf->e_ehsize);     // buffer to store information about the current program header
    vfsRead(fd, phdr, elf->e_ehsize, elf->e_phoff); // read first header

    // todo: if we find the .bss section zero it to gurantee the app that it's clear
    for (int i = 0; i < elf->e_phnum; i++) // iterate over every program header
    {
        if (phdr->p_type == PT_LOAD) // section to be loaded
        {
#ifdef K_ELF_DEBUG
            logDbg(LOG_SERIAL_ONLY, "elf: phdr at virtual 0x%p (physical 0x%p) with size %d bytes", phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz);
#endif
            vfsRead(fd, (void *)((uint64_t)buffer + phdr->p_vaddr - TASK_BASE_ADDRESS), phdr->p_memsz, phdr->p_offset); // copy the program header to the buffer using the vfs
        }

        vfsRead(fd, phdr, elf->e_ehsize, elf->e_phoff + i); // read next program header
    }

    blkDeallocate(phdr); // clean up

    char *cwd = pmmPage();
    memcpy(cwd, path, strlen(path)); // copy the path
    for (int i = strlen(cwd) - 1; cwd[i] != '/'; cwd[i--] = '\0')
        ; // step back to last delimiter (removes file name)

    sched_task_t *task = schedAdd(path, (void *)elf->e_entry - TASK_BASE_ADDRESS, K_STACK_SIZE, buffer, fdSize, 0, cwd, argc, argv, true, driver); // add the task                                                                                                                         // free
    pmmDeallocate(cwd);
    blkDeallocate(elf);
    return task; // return the task
}