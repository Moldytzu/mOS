#include <vmm.h>
#include <pmm.h>
#include <serial.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>

struct vmm_page_table *baseTable;

struct vmm_index vmmIndex(uint64_t virtualAddress)
{
    struct vmm_index index;
    virtualAddress >>= 12;
    index.P = virtualAddress & 0x1FF;                    // get the first 9 bits
    index.PT = (virtualAddress >> 9) & 0x1FF;            // get the next 9 bits
    index.PD = (virtualAddress >> 9 >> 9) & 0x1FF;       // get the next 9 bits
    index.PDP = (virtualAddress >> 9 >> 9 >> 9) & 0x1FF; // get the next 9 bits
    return index;
}

void vmmInit()
{
    void *bootloaderTable = NULL;
    iasm("mov %%cr3, %%rax"
         : "=a"(bootloaderTable)); // get bootloader's paging table

    baseTable = vmmCreateTable(true); // create the base table with hhdm
    vmmSwap(baseTable);               // swap the table
}

bool vmmGetFlag(uint64_t *entry, uint8_t flag)
{
    return *entry & (1 << flag); // get flag
}

void vmmSetFlag(uint64_t *entry, uint8_t flag, bool value)
{
    if (value)
    {
        *entry |= (1 << flag); // set flag
        return;                // return
    }

    *entry &= ~(1 << flag); // unset flag
}

uint64_t vmmGetAddress(uint64_t *entry)
{
    return (*entry & 0x000FFFFFFFFFF000) >> 12; // get the address from entry
}

void vmmSetAddress(uint64_t *entry, uint64_t address)
{
    *entry &= 0xfff0000000000fff;             // clear address field
    *entry |= (address & 0xFFFFFFFFFF) << 12; // set the address field
}

void vmmSetFlags(struct vmm_page_table *table, struct vmm_index index, bool user, bool rw)
{
    struct vmm_page_table *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);       // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);   // userspace
    table->entries[index.PDP] = currentEntry;          // write the entry in the table

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

void vmmMap(struct vmm_page_table *table, void *virtualAddress, void *physicalAddress, bool user, bool rw)
{
    struct vmm_index index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    struct vmm_page_table *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pdp = mmAllocatePage();                             // allocate table
        memset64(pdp, 0, VMM_PAGE / sizeof(uint64_t));      // clear it
        vmmSetAddress(&currentEntry, (uint64_t)pdp >> 12);  // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        table->entries[index.PDP] = currentEntry;           // write the entry in the table
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

void vmmUnmap(struct vmm_page_table *table, void *virtualAddress)
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

void *vmmGetBaseTable()
{
    return baseTable;
}

void vmmSwap(void *newTable)
{
    iasm("mov %0, %%cr3" ::"r"(newTable));
}

void *vmmGetPhys(struct vmm_page_table *table, void *virtualAddress)
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

    currentEntry = pt->entries[index.P];         // index p
    return (void *)vmmGetAddress(&currentEntry); // get the address
}

struct pack vmm_page_table *vmmCreateTable(bool full)
{
    // create a new table to use as a base for everything
    register void *newTable = mmAllocatePage();         // allocate a page for the new table
    memset64(newTable, 0, VMM_PAGE / sizeof(uint64_t)); // clear the paging table

    struct stivale2_struct_tag_kernel_base_address *kaddr = bootloaderGetKernelAddr(); // get kernel address
    struct stivale2_struct_tag_pmrs *pmrs = bootloaderGetPMRS();                       // get pmrs
    struct stivale2_struct_tag_framebuffer *framebuffer = bootloaderGetFramebuf();     // get framebuffer address

#ifdef K_VMM_DEBUG
    uint64_t a = mmGetTotal().available;
#endif

    // map PMRs
    for (size_t i = 0; i < pmrs->entries; i++)
    {
        register struct stivale2_pmr currentPMR = pmrs->pmrs[i];
        for (size_t j = 0; j < currentPMR.length; j += VMM_PAGE)
            vmmMap(newTable, (void *)currentPMR.base + j, (void *)kaddr->physical_base_address + (currentPMR.base - kaddr->virtual_base_address) + j, false, true);
    }

    if (full)
    {
        register uint64_t total = mmGetTotal().total + 0xFFFF0;

        for (uint64_t i = 0; i < total; i += VMM_PAGE) // map entire physical memory range
            vmmMap(newTable, (void *)i, (void *)i, false, true);

        for (uint64_t i = 0; i < total; i += VMM_PAGE) // map hhdm
            vmmMap(newTable, (uint64_t)i + bootloaderGetHHDM(), (void *)i, false, true);

        for (uint64_t i = framebuffer->framebuffer_addr - (uint64_t)bootloaderGetHHDM(); i < framebuffer->framebuffer_addr - (uint64_t)bootloaderGetHHDM() + (framebuffer->framebuffer_pitch * framebuffer->framebuffer_height); i += VMM_PAGE) // map framebuffer
            vmmMap(newTable, (uint64_t)i + bootloaderGetHHDM(), (void *)i, false, true);
    }
    else
        for (uint64_t i = 0; i < 16 * 1024 * 1024; i += VMM_PAGE) // map only 16 MB of RAM
            vmmMap(newTable, (void *)i, (void *)i, false, true);

    vmmMap(newTable, newTable, newTable, false, true); // map the table

#ifdef K_IDT_IST
    vmmMap(newTable, (void*)tssGet()->ist[0], (void*)tssGet()->ist[0], true, true); // map ists
    vmmMap(newTable, (void*)tssGet()->ist[1], (void*)tssGet()->ist[1], true, true); // map ist
#endif

#ifdef K_VMM_DEBUG
    serialWrite("vmm: wasted ");
    serialWrite(to_string(toKB((uint64_t)(a - mmGetTotal().available))));
    serialWrite(" KB on a page table\n");
#endif

    return newTable; // return the created table
}