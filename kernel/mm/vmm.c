#include <mm/vmm.h>
#include <mm/pmm.h>
#include <drv/serial.h>
#include <fw/bootloader.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>

bool pml5 = false;
struct vmm_page_table *baseTable;

// initialize the virtual memory manager
void vmmInit()
{
    pml5 = bootloaderProbePML5();     // check if pml5 is supported
    baseTable = vmmCreateTable(true); // create the base table with hhdm
    vmmSwap(baseTable);               // swap the table
}

// set flags of some entries given by the indices
void optimize vmmSetFlags(struct vmm_page_table *table, struct vmm_index index, bool user, bool rw)
{
    struct vmm_page_table *pml4, *pdp, *pd, *pt;
    uint64_t currentEntry;
    pml4 = table;

    currentEntry = pml4->entries[index.PDP];         // index pdp
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);     // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user); // userspace
    pml4->entries[index.PDP] = currentEntry;         // write the entry in the table

    pdp = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pdp->entries[index.PD];                               // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                         // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);                     // userspace
    pdp->entries[index.PD] = currentEntry;                               // write the entry in the table

    pd = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pd->entries[index.PT];                               // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                        // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);                    // userspace
    pd->entries[index.PT] = currentEntry;                               // write the entry in the table

    pt = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pt->entries[index.P];                                // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                        // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);                    // userspace
    pt->entries[index.P] = currentEntry;                                // write the entry in the table
}

// map a virtual address to a physical address in a page table
void optimize vmmMap(struct vmm_page_table *table, void *virtualAddress, void *physicalAddress, bool user, bool rw)
{
    struct vmm_index index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    struct vmm_page_table *pml4, *pdp, *pd, *pt;
    uint64_t currentEntry;

    pml4 = table;

    currentEntry = pml4->entries[index.PDP];           // index pdp
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pdp = mmAllocatePage();                             // allocate table
        memset64(pdp, 0, VMM_PAGE / sizeof(uint64_t));      // clear it
        vmmSetAddress(&currentEntry, (uint64_t)pdp >> 12);  // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pml4->entries[index.PDP] = currentEntry;            // write the entry in the table
    }
    else
        pdp = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pdp->entries[index.PD];             // index pd
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pd = mmAllocatePage();                              // allocate table
        memset64(pd, 0, VMM_PAGE / sizeof(uint64_t));       // clear it
        vmmSetAddress(&currentEntry, (uint64_t)pd >> 12);   // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pdp->entries[index.PD] = currentEntry;              // write the entry in the table
    }
    else
        pd = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pd->entries[index.PT];              // index pt
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pt = mmAllocatePage();                              // allocate table
        memset64(pt, 0, VMM_PAGE / sizeof(uint64_t));       // clear it
        vmmSetAddress(&currentEntry, (uint64_t)pt >> 12);   // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pd->entries[index.PT] = currentEntry;               // write the entry in the table
    }
    else
        pt = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pt->entries[index.P];                           // index p
    vmmSetAddress(&currentEntry, (uint64_t)physicalAddress >> 12); // set the address to the physical one
    vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true);            // present
    pt->entries[index.P] = currentEntry;                           // write the entry in the table

    vmmSetFlags(table, index, user, rw); // set the flags
}

// unmap a virtual address
void optimize vmmUnmap(struct vmm_page_table *table, void *virtualAddress)
{
    struct vmm_index index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    struct vmm_page_table *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    pdp = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pdp->entries[index.PD]; // index pd
    pd = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pd->entries[index.PT]; // index pt
    pt = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pt->entries[index.P];                 // index p
    vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, false); // unvalidate page
    pt->entries[index.P] = currentEntry;                 // write the entry in the table
}

// get the base table aka kernel table
void *vmmGetBaseTable()
{
    return baseTable;
}

// get physical address of a virtual address
void optimize *vmmGetPhys(struct vmm_page_table *table, void *virtualAddress)
{
    // get physical memory address form virtual memory address
    struct vmm_index index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    struct vmm_page_table *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP];                   // index pdp
    pdp = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pdp->entries[index.PD];                              // index pd
    pd = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pd->entries[index.PT];                               // index pt
    pt = (struct vmm_page_table *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pt->entries[index.P];                                                              // index p
    return (void *)(vmmGetAddress(&currentEntry) * VMM_PAGE + ((uint64_t)virtualAddress % VMM_PAGE)); // get the address
}

// create a new table
struct pack vmm_page_table optimize *vmmCreateTable(bool full)
{
    // create a new table to use as a base for everything
    register void *newTable = mmAllocatePage();         // allocate a page for the new table
    memset64(newTable, 0, VMM_PAGE / sizeof(uint64_t)); // clear the paging table

    struct stivale2_struct_tag_kernel_base_address *kaddr = bootloaderGetKernelAddr(); // get kernel address
    struct stivale2_struct_tag_pmrs *pmrs = bootloaderGetPMRS();                       // get pmrs
    struct stivale2_struct_tag_framebuffer *framebuffer = bootloaderGetFramebuf();     // get framebuffer address
    struct stivale2_struct_tag_memmap *map = bootloaderGetMemMap();                    // get the memory map

#ifdef K_VMM_DEBUG
    uint64_t a = mmGetTotal().available;
#endif

    // map PMRs
    for (size_t i = 0; i < pmrs->entries; i++)
    {
        register struct stivale2_pmr currentPMR = pmrs->pmrs[i];
        for (size_t j = 0; j < currentPMR.length; j += VMM_PAGE)
            vmmMap(newTable, (void *)currentPMR.base + j, (void *)kaddr->physical_base_address + (currentPMR.base - kaddr->virtual_base_address) + j, false, (bool)(currentPMR.permissions & STIVALE2_PMR_WRITABLE));
    }

    if (full)
    {
        uint64_t hhdm = (uint64_t)bootloaderGetHHDM();

        // map all memory map entries
        for (uint64_t i = 0; i < map->entries; i++)
        {
            for (uint64_t j = 0; j < map->memmap[i].length; j += 4096)
            {
                vmmMap(newTable, (void *)map->memmap[i].base + j, (void *)map->memmap[i].base + j, false, true);
                vmmMap(newTable, (void *)map->memmap[i].base + j + hhdm, (void *)map->memmap[i].base + j, false, true);
            }
        }
    }

    vmmMap(newTable, newTable, newTable, false, true);                 // map the table
    vmmMap(newTable, (void *)tssGet(), (void *)tssGet(), false, true); // map the tss
    vmmMap(newTable, (void *)gdtGet(), (void *)gdtGet(), false, true); // map the gdt

#ifdef K_IDT_IST
    vmmMap(newTable, (void *)tssGet()->ist[0], (void *)tssGet()->ist[0], false, true); // kernel ist
    vmmMap(newTable, (void *)tssGet()->ist[1], (void *)tssGet()->ist[1], false, true); // user ist
#endif

#ifdef K_VMM_DEBUG
    printks("vmm: wasted %d KB on a page table\n\r", toKB(a - mmGetTotal().available));
#endif

    return newTable; // return the created table
}

// free a table
void optimize vmmDestroy(struct vmm_page_table *table)
{
#ifdef K_VMM_DEBUG
    printks("vmm: destroying page table at 0x%p\n\r", table);
#endif
    for (int i = 0; i < 512; i++)
    {
        uint64_t address = table->entries[i];

        if (!vmmGetFlag(&address, VMM_ENTRY_PRESENT)) // we look for the present entries only
            continue;

        mmDeallocatePage((void *)vmmGetAddress(&address)); // deallocate it
    }

    mmDeallocatePage(table);
}