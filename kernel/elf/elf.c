#include <elf/elf.h>
#include <elf/elfabi.h>
#include <fs/vfs.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <sched/scheduler.h>

// load an elf
struct sched_task *elfLoad(const char *path, int argc, char **argv, bool driver)
{
    uint64_t fd = vfsOpen(path);                   // open the file
    uint64_t fdSize = vfsSize(fd);                 // get the size
    Elf64_Ehdr *elf = pmmPages(fdSize / 4096 + 1); // allocate the raw elf
    if (!elf)                                      // return if we didn't get the header
    {
        pmmDeallocatePages(elf, fdSize / 4096 + 1);
        return false;
    }

    zero(elf, fdSize); // clear the area

    vfsRead(fd, elf, fdSize, 0); // read the elf

    // check compatibility
    if (elf->e_ident[EI_CLASS] != ELFCLASS64 || elf->e_ident[EI_DATA] != ELFDATA2LSB || elf->e_type != ET_EXEC || elf->e_machine != EM_X86_64 || elf->e_version != EV_CURRENT)
        return false;

#ifdef K_ELF_DEBUG
    printks("elf: found %s at 0x%p with the entry offset at 0x%p\n\r", path, elf, elf->e_entry - TASK_BASE_ADDRESS);
#endif

    void *buffer = pmmPages(fdSize / VMM_PAGE + 1); // allocate the buffer for the sections

    zero(buffer, fdSize); // clear the sections

    Elf64_Phdr *phdr = (Elf64_Phdr *)((uint64_t)elf + elf->e_phoff); // point to the first program header

    for (int i = 0; i < elf->e_phnum; i++, phdr++) // iterate over every program header
    {
        if (phdr->p_type == PT_LOAD) // section to be loaded
        {
#ifdef K_ELF_DEBUG
            printks("elf: phdr at virtual 0x%p (physical 0x%p) with size %d bytes\n\r", phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz);
#endif
            memcpy64((void *)((uint64_t)buffer + phdr->p_vaddr - TASK_BASE_ADDRESS), (void *)((uint64_t)elf + phdr->p_offset), phdr->p_memsz / sizeof(uint64_t)); // copy the program header to the buffer
        }
    }

    pmmDeallocatePages(elf, fdSize / 4096 + 1); // free the elf

    char *cwd = blkBlock(strlen(path));
    zero(cwd, strlen(path)); // clear the string
    memcpy(cwd, path, strlen(path));
    for (int i = strlen(cwd) - 1; cwd[i] != '/'; cwd[i--] = '\0')
        ; // step back to last delimiter

    struct sched_task *task = schedulerAdd(path, (void *)elf->e_entry - TASK_BASE_ADDRESS, K_STACK_SIZE, buffer, fdSize, 0, cwd, argc, argv, true, driver); // add the task
    blkDeallocate(cwd, strlen(path));                                                                                                                       // free
    return task;                                                                                                                                            // return the task
}