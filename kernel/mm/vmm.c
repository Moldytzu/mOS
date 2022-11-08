#include <mm/vmm.h>
#include <mm/pmm.h>
#include <drv/serial.h>
#include <fw/bootloader.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>

struct vmm_page_table *baseTable;

// initialize the virtual memory manager
void vmmInit()
{
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
        pdp = pmmPage();                             // allocate table
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
        pd = pmmPage();                              // allocate table
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
        pt = pmmPage();                              // allocate table
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
#ifdef K_VMM_DEBUG
    uint64_t a = pmmTotal().available;
#endif

    // create a new table to use as a base for everything
    register void *newTable = pmmPage(); // allocate a page for the new table
    memset(newTable, 0, VMM_PAGE);              // clear the page table

    struct limine_memmap_response *memMap = bootloaderGetMemoryMap();
    uint64_t hhdm = (uint64_t)bootloaderGetHHDM();
    struct limine_kernel_address_response *kaddr = bootloaderGetKernelAddress();

    // map the system tables as kernel rw
    vmmMap(newTable, newTable, newTable, false, true);                 // page table
    vmmMap(newTable, (void *)tssGet(), (void *)tssGet(), false, true); // tss
    vmmMap(newTable, (void *)gdtGet(), (void *)gdtGet(), false, true); // gdt

#ifdef K_IDT_IST
    vmmMap(newTable, (void *)tssGet()->ist[0], (void *)tssGet()->ist[0], false, true); // kernel ist
    vmmMap(newTable, (void *)tssGet()->ist[1], (void *)tssGet()->ist[1], false, true); // user ist
#endif

    // map memory map entries as kernel rw
    for (size_t i = 0; i < memMap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memMap->entries[i];

        if (entry->type != LIMINE_MEMMAP_KERNEL_AND_MODULES && !full) // don't map any type of memory besides kernel
            continue;

        if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
        {
            for (size_t i = 0; i < entry->length; i += 4096)
                vmmMap(newTable, (void *)(kaddr->virtual_base + i), (void *)(kaddr->physical_base + i), false, true);
        }
        for (size_t i = 0; i < entry->length; i += 4096)
        {
            vmmMap(newTable, (void *)(entry->base + i), (void *)(entry->base + i), false, true);
            vmmMap(newTable, (void *)(entry->base + i + hhdm), (void *)(entry->base + i), false, true);
        }
    }

#ifdef K_VMM_DEBUG
    printks("vmm: wasted %d KB on a page table\n\r", toKB(a - pmmTotal().available));
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

        pmmDeallocate((void *)vmmGetAddress(&address)); // deallocate it
    }

    pmmDeallocate(table);
}