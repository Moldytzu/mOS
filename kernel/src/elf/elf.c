#include <elf/elf.h>
#include <elf/elfabi.h>
#include <fs/vfs.h>
#include <mm/pmm.h>
#include <mm/blk.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

#define ELF_MAGIC "\x7F" \
                  "ELF"

bool elfIsCompatible(Elf64_Ehdr *elf)
{
    return memcmp(elf, ELF_MAGIC, 4) == 0 &&       // check magic
           elf->e_ident[EI_CLASS] == ELFCLASS64 && // check for object size
           elf->e_ident[EI_DATA] == ELFDATA2LSB && // check encoding
           elf->e_type == ET_EXEC &&               // check type
           elf->e_machine == EM_X86_64 &&          // check architecture
           elf->e_version == EV_CURRENT;           // check version
}

// load a static elf binary as a userspace task
sched_task_t *elfLoad(const char *path, int argc, char **argv, bool driver)
{
    uint64_t fd = vfsOpen(path);                    // open the file
    uint64_t fdSize = vfsSize(fd);                  // get the size
    Elf64_Ehdr *elf = blkBlock(sizeof(Elf64_Ehdr)); // allocate the elf header

    vfsRead(fd, elf, sizeof(Elf64_Ehdr), 0); // read the header

    // check compatibility
    if (!elfIsCompatible(elf))
    {
        // clean up
        blkDeallocate(elf);
        vfsClose(fd);
        return NULL;
    }

#ifdef K_ELF_DEBUG
    logDbg(LOG_SERIAL_ONLY, "elf: found %s at 0x%p with the entry offset at 0x%p", path, elf, elf->e_entry - TASK_BASE_ADDRESS);
#endif

    uint64_t virtualBase = UINT64_MAX; // base virtual address of the executable in memory

    // determine in-memory size of executable and virtual base (pass 1)
    Elf64_Phdr *phdr = blkBlock(elf->e_phentsize);     // buffer to store information about the current program header
    vfsRead(fd, phdr, elf->e_phentsize, elf->e_phoff); // read first header

    size_t memsz = 0;
    for (int i = 0; i < elf->e_phnum; i++) // iterate over every program header
    {
        if (phdr->p_type == PT_LOAD) // program header to be loaded
        {
#ifdef K_ELF_DEBUG
            logDbg(LOG_SERIAL_ONLY, "elf: phdr at virtual 0x%p (physical 0x%p) with size %d bytes", phdr->p_vaddr, phdr->p_paddr, phdr->p_memsz);
#endif
            memsz += phdr->p_memsz;

            if (phdr->p_vaddr < virtualBase) // find lowest address possible
                virtualBase = phdr->p_vaddr;
        }

        vfsRead(fd, phdr, elf->e_phentsize, elf->e_phoff + i * elf->e_phentsize); // read next program header
    }
    memsz = align(memsz, PMM_PAGE); // align to page size

    // fixme: do memory availability check
    if (virtualBase < TASK_BASE_SWITCH_TO_BUFFER + 2 * VMM_PAGE /*minimum possible address space*/ || virtualBase + memsz > 0x800000000000 /*outside lower half*/) // boundary check for addressing space usage
    {
        logError("elf: failing to load an executable with virtual base at 0x%p", virtualBase);

        // clean up
        blkDeallocate(elf);
        blkDeallocate(phdr);

        return NULL;
    }

    // get string table
    Elf64_Shdr *stringTableHeader = blkBlock(elf->e_shentsize);
    vfsRead(fd, stringTableHeader, elf->e_shentsize, elf->e_shoff + elf->e_shstrndx * elf->e_shentsize); // e_shstrnidx is the index of the section header that holds the strings table

    logDbg(LOG_SERIAL_ONLY, "elf: string table is at 0x%p (section %d)", elf->e_shoff + elf->e_shstrndx * elf->e_shentsize, elf->e_shstrndx);

    // parse section headers
    Elf64_Shdr *shdr = blkBlock(elf->e_shentsize);

    vfsRead(fd, shdr, elf->e_shentsize, elf->e_shoff); // read first header

    for (int i = 0; i < elf->e_shnum; i++) // iterate over every section header
    {
        logDbg(LOG_SERIAL_ONLY, "elf: section %d is at 0x%p and has name %d", i, shdr->sh_addr, shdr->sh_name);
        vfsRead(fd, shdr, elf->e_shentsize, elf->e_shoff + i * elf->e_shentsize); // read next header
    }

    blkDeallocate(shdr);
    blkDeallocate(stringTableHeader);

    // load program headers (pass 3)
    vfsRead(fd, phdr, elf->e_phentsize, elf->e_phoff); // read first header
    void *buffer = pmmPages(memsz / PMM_PAGE);         // allocate the buffer for the sections

    for (int i = 0; i < elf->e_phnum; i++) // iterate over every program header
    {
        if (phdr->p_type == PT_LOAD) // program header to be loaded
        {
            vfsRead(fd, (void *)((uint64_t)buffer + phdr->p_vaddr - virtualBase), phdr->p_filesz, phdr->p_offset); // copy the program header to the buffer using the vfs
        }

        vfsRead(fd, phdr, elf->e_phentsize, elf->e_phoff + i); // read next program header
    }

    blkDeallocate(phdr); // clean up

    // copy cwd
    char *cwd = pmmPage();
    memcpy(cwd, path, strlen(path)); // copy the path
    for (int i = strlen(cwd) - 1; cwd[i] != '/'; cwd[i--] = '\0')
        ; // step back to last delimiter (removes file name)

    // add the task
    sched_task_t *task = schedAdd(path, (void *)elf->e_entry - virtualBase, buffer, memsz, virtualBase, 0, cwd, argc, argv, true, driver);

    // clean up
    pmmDeallocate(cwd);
    blkDeallocate(elf);
    return task; // return the task
}