#pragma once
#include <misc/utils.h>
#include <cpu/control.h>

#define VMM_ENTRY_PRESENT 0
#define VMM_ENTRY_RW 1
#define VMM_ENTRY_USER 2
#define VMM_ENTRY_WRITE_THROUGH 3
#define VMM_ENTRY_CACHE 4
#define VMM_ENTRY_ACCESSED 5
#define VMM_ENTRY_DIRTY 6
#define VMM_ENTRY_HUGE_PAGES 7
#define VMM_ENTRY_NO_EXECUTE 63

#define VMM_PAGE 4096

struct pack vmm_index
{
    uint64_t PML4;
    uint64_t PDP;
    uint64_t PD;
    uint64_t PT;
    uint64_t P;
};

struct pack vmm_page_table
{
    uint64_t entries[512];
};

// operations on entries
ifunc optimize bool vmmGetFlag(uint64_t *entry, uint8_t flag)
{
    return *entry & (1 << flag); // get flag
}

ifunc optimize void vmmSetFlag(uint64_t *entry, uint8_t flag, bool value)
{
    if (value)
    {
        *entry |= (1 << flag); // set flag
        return;                // return
    }

    *entry &= ~(1 << flag); // unset flag
}

// get the address of an entry
ifunc optimize uint64_t vmmGetAddress(uint64_t *entry)
{
    return (*entry & 0x000FFFFFFFFFF000) >> 12; // get the address from entry
}

// set the address of an entry
ifunc optimize void vmmSetAddress(uint64_t *entry, uint64_t address)
{
    *entry &= 0xfff0000000000fff;             // clear address field
    *entry |= (address & 0xFFFFFFFFFF) << 12; // set the address field
}

// indexer
ifunc optimize struct vmm_index vmmIndex(uint64_t virtualAddress)
{
    struct vmm_index index;
    index.P = (virtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
    index.PT = (virtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
    index.PD = (virtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
    index.PDP = (virtualAddress & ((uint64_t)0x1FF << 39)) >> 39;
    index.PML4 = (virtualAddress & ((uint64_t)0x1FF << 48)) >> 48;
    return index;
}

// misc
void vmmInit();
struct pack vmm_page_table *vmmCreateTable(bool full);

// swap the page table
ifunc void vmmSwap(void *newTable)
{
    controlLoadCR3((uint64_t)newTable); // cr3 is the register that holds the table
}

// mapping
void vmmSetFlags(struct vmm_page_table *table, struct vmm_index index, bool user, bool rw);
void vmmMap(struct vmm_page_table *table, void *virtualAddress, void *physicalAddress, bool user, bool rw);
void vmmUnmap(struct vmm_page_table *table, void *virtualAddress);
void *vmmGetBaseTable();
void *vmmGetPhys(struct vmm_page_table *table, void *virtualAddress);
void vmmDestroy(struct vmm_page_table *table);