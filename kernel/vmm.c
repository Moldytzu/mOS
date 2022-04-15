#include <vmm.h>
#include <pmm.h>
#include <serial.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <control.h>

struct vmm_page_table *baseTable;

// get indices in the page table of a virtual address
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

// initialize the virtual memory manager
void vmmInit()
{
    baseTable = vmmCreateTable(true); // create the base table with hhdm
    vmmSwap(baseTable);               // swap the table
}

// get flag of an entry
bool vmmGetFlag(uint64_t *entry, uint8_t flag)
{
    return *entry & (1 << flag); // get flag
}

// set flag of an entry
void vmmSetFlag(uint64_t *entry, uint8_t flag, bool value)
{
    if (value)
    {
        *entry |= (1 << flag); // set flag
        return;                // return
    }

    *entry &= ~(1 << flag); // unset flag
}

// get address of an entry
uint64_t vmmGetAddress(uint64_t *entry)
{
    return (*entry & 0x000FFFFFFFFFF000) >> 12; // get the address from entry
}

// set address of an entry
void vmmSetAddress(uint64_t *entry, uint64_t address)
{
    *entry &= 0xfff0000000000fff;             // clear address field
    *entry |= (address & 0xFFFFFFFFFF) << 12; // set the address field
}

// set flags of some entries given by the indices
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

// map a virtual address to a physical address in a page table 
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

// unmap a virtual address
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

// get the base table aka kernel table
void *vmmGetBaseTable()
{
    return baseTable;
}

// swap the current page table with a new one
void vmmSwap(void *newTable)
{
    controlLoadCR3((uint64_t)newTable); // cr3 is the register that holds the table
}

// get physical address of a virtual address
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

// create a new table
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
        uint64_t hhdm = (uint64_t)bootloaderGetHHDM();
        register uint64_t total = mmGetTotal().total + 0xFFFF0;

        for (uint64_t i = 0; i < total; i += VMM_PAGE) // identity map entire physical memory range
        {
            vmmMap(newTable, (void *)i, (void *)i, false, true);
            vmmMap(newTable, (void *)i + hhdm, (void *)i, false, true); // map in hhdm
        }

        for (uint64_t i = framebuffer->framebuffer_addr - hhdm; i < framebuffer->framebuffer_addr - hhdm + (framebuffer->framebuffer_pitch * framebuffer->framebuffer_height); i += VMM_PAGE) // map framebuffer
            vmmMap(newTable, (void *)i + hhdm, (void *)i, false, true);
    }
    else
        for (uint64_t i = 0; i < 16 * 1024 * 1024; i += VMM_PAGE) // map only 16 MB of RAM
            vmmMap(newTable, (void *)i, (void *)i, false, true);

    vmmMap(newTable, newTable, newTable, false, true);                 // map the table
    vmmMap(newTable, (void *)tssGet(), (void *)tssGet(), false, true); // map the tss
    vmmMap(newTable, (void *)gdtGet(), (void *)gdtGet(), false, true); // map the gdt

#ifdef K_IDT_IST
    vmmMap(newTable, (void *)tssGet()->ist[0], (void *)tssGet()->ist[0], false, true); // kernel ist
    vmmMap(newTable, (void *)tssGet()->ist[1], (void *)tssGet()->ist[1], true, true);  // user ist
#endif

#ifdef K_VMM_DEBUG
    printkc("vmm: wasted %d KB on a page table\n\r", toKB(a - mmGetTotal().available));
#endif

    return newTable; // return the created table
}