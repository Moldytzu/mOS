#include <elf/elf.h>
#include <elf/elfabi.h>
#include <fs/vfs.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/scheduler.h>

// load an elf
struct sched_task *elfLoad(const char *path, int argc, char **argv)
{
    uint64_t fd = vfsOpen(path);           // open the file
    Elf64_Ehdr *elf = malloc(vfsSize(fd)); // allocate the raw elf
    if (!elf)                              // return if we didn't get the header
    {
        free(elf); // free it
        return false;
    }

    vfsRead(fd, elf, vfsSize(fd), 0); // read the elf

    // check compatibility
    if (elf->e_ident[EI_CLASS] != ELFCLASS64 || elf->e_ident[EI_DATA] != ELFDATA2LSB || elf->e_type != ET_EXEC || elf->e_machine != EM_X86_64 || elf->e_version != EV_CURRENT)
        return false;

#ifdef K_ELF_DEBUG
    printks("elf: found %s at 0x%p with the entry offset at 0x%p\n\r", path, elf, elf->e_entry - TASK_BASE_ADDRESS);
#endif

    void *buffer = pmmPages(vfsSize(fd) / VMM_PAGE + 1); // allocate the buffer for the sections

    memset64(buffer, 0, vfsSize(fd) / sizeof(uint64_t));

    Elf64_Phdr *phdr = (Elf64_Phdr *)((uint64_t)elf + elf->e_phoff); // get the program headers from the offset

    for (int i = 0; i < elf->e_phnum; i++, phdr++) // iterate over every program header
    {
        if (phdr->p_type == PT_LOAD) // section to be loaded
        {
#ifdef K_ELF_DEBUG
            printks("elf: phdr at virtual 0x%p (physical 0x%p) with size %d bytes\n\r", phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz);
#endif
            memcpy64((void *)((uint64_t)buffer + phdr->p_vaddr - TASK_BASE_ADDRESS), (void *)((uint64_t)elf + phdr->p_offset), phdr->p_memsz / sizeof(uint64_t));
        }
    }

    free(elf); // free the elf

    char *cwd = malloc(strlen(path));
    memcpy(cwd, path, strlen(path));
    for (int i = strlen(cwd) - 1; cwd[i] != '/'; cwd[i--] = '\0')
        ; // step back to last delimiter

    struct sched_task *task = schedulerAdd(path, (void *)elf->e_entry - TASK_BASE_ADDRESS, VMM_PAGE, buffer, vfsSize(fd), 0, cwd, argc, argv, true); // add the task
    free(cwd);                                                                                                                                       // free
    return task;                                                                                                                                     // return the task
}