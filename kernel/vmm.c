#include <vmm.h>
#include <pmm.h>

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

    baseTable = mmAllocatePage();                                  // allocate a page for the base table
    memcpy64(baseTable, bootloaderTable, 4096 / sizeof(uint64_t)); // copy bootloader's paging table over our base table

    iasm("mov %0, %%cr3" ::"r"(baseTable)); // set cr3 to our "new" table

    // identity map all the memory pools
    struct mm_pool * pools = mmGetPools();
    for (size_t i = 0; pools[i].total != UINT64_MAX; i++)
    {
        for(size_t j = 0; j < pools[i].total; j += 4096)
            vmmMap(baseTable, pools[i].base + j, pools[i].base + j);
    }
}

bool vmmGetFlag(uint64_t entry, uint8_t flag)
{
    return entry & (1 << flag); // get flag
}

void vmmSetFlag(uint64_t entry, uint8_t flag, bool value)
{
    if (value)
    {
        entry |= (1 << flag); // set flag
        return;               // return
    }

    entry &= ~(1 << flag); // unset flag
}

uint64_t vmmGetAddress(uint64_t entry)
{
    return (entry & 0x000FFFFFFFFFF000) >> 12; // get the address from entry
}

void vmmSetAddress(uint64_t entry, uint64_t address)
{
    entry &= 0xfff0000000000fff;             // clear address field
    entry |= (address & 0xFFFFFFFFFF) << 12; // set the address field
}

void vmmMap(struct vmm_page_table *table, void *virtualAddress, void *physicalAddress)
{
    struct vmm_index index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    struct vmm_page_table *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    if (!vmmGetFlag(currentEntry, VMM_ENTRY_PRESENT))  // if there isn't any page present there, we generate it
    {
        pdp = mmAllocatePage();                            // allocate table
        memset64(pdp, 0, 4096 / sizeof(uint64_t));         // clear it
        vmmSetAddress(currentEntry, (uint64_t)pdp >> 12);  // set it's address
        vmmSetFlag(currentEntry, VMM_ENTRY_PRESENT, true); // present
        vmmSetFlag(currentEntry, VMM_ENTRY_RW, true);      // rewrite
        table->entries[index.PDP] = currentEntry;          // write the entry in the table
    }
    else
        pdp = (struct vmm_page_table *)(vmmGetAddress(currentEntry) << 12); // continue

    currentEntry = pdp->entries[index.PD];            // index pd
    if (!vmmGetFlag(currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pd = mmAllocatePage();                             // allocate table
        memset64(pd, 0, 4096 / sizeof(uint64_t));          // clear it
        vmmSetAddress(currentEntry, (uint64_t)pd >> 12);   // set it's address
        vmmSetFlag(currentEntry, VMM_ENTRY_PRESENT, true); // present
        vmmSetFlag(currentEntry, VMM_ENTRY_RW, true);      // rewrite
        pdp->entries[index.PD] = currentEntry;             // write the entry in the table
    }
    else
        pd = (struct vmm_page_table *)(vmmGetAddress(currentEntry) << 12); // continue

    currentEntry = pd->entries[index.PT];             // index pt
    if (!vmmGetFlag(currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pt = mmAllocatePage();                             // allocate table
        memset64(pt, 0, 4096 / sizeof(uint64_t));          // clear it
        vmmSetAddress(currentEntry, (uint64_t)pt >> 12);   // set it's address
        vmmSetFlag(currentEntry, VMM_ENTRY_PRESENT, true); // present
        vmmSetFlag(currentEntry, VMM_ENTRY_RW, true);      // rewrite
        pd->entries[index.PT] = currentEntry;              // write the entry in the table
    }
    else
        pt = (struct vmm_page_table *)(vmmGetAddress(currentEntry) << 12); // continue

    currentEntry = pt->entries[index.P];                          // index p
    vmmSetAddress(currentEntry, (uint64_t)physicalAddress >> 12); // set the address to the physical one
    vmmSetFlag(currentEntry, VMM_ENTRY_PRESENT, true);            // present
    vmmSetFlag(currentEntry, VMM_ENTRY_RW, true);                 // rewrite
    pt->entries[index.P] = currentEntry;                          // write the entry in the table
}

void *vmmGetBaseTable()
{
    return baseTable;
}