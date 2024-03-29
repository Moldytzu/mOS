#pragma once
#include <misc/utils.h>
#include <cpu/control.h>
#include <cpu/tlb.h>

#define VMM_ENTRY_PRESENT (1 << 0)
#define VMM_ENTRY_RW (1 << 1)
#define VMM_ENTRY_RO 0
#define VMM_ENTRY_USER (1 << 2)
#define VMM_ENTRY_WRITE_THROUGH (1 << 3)
#define VMM_ENTRY_CACHE_DISABLE (1 << 4)
#define VMM_ENTRY_ACCESSED (1 << 5)
#define VMM_ENTRY_DIRTY (1 << 6)
#define VMM_ENTRY_HUGE_PAGES (1 << 7)
#define VMM_ENTRY_NO_EXECUTE (1 << 63)

#define VMM_PAGE 4096

pstruct
{
    uint64_t PML4;
    uint64_t PDP;
    uint64_t PD;
    uint64_t PT;
    uint64_t P;
}
vmm_index_t;

pstruct
{
    uint64_t entries[512];
}
vmm_page_table_t;

// set the address of an entry
ifunc void vmmSetAddress(uint64_t *entry, uint64_t address)
{
    *entry &= 0xfff0000000000fff;             // clear address field
    *entry |= (address & 0xFFFFFFFFFF) << 12; // set the address field
}

// indexer
ifunc vmm_index_t vmmIndex(uint64_t virtualAddress)
{
    vmm_index_t index;
    index.P = (virtualAddress & ((uint64_t)0x1FF << 12)) >> 12;
    index.PT = (virtualAddress & ((uint64_t)0x1FF << 21)) >> 21;
    index.PD = (virtualAddress & ((uint64_t)0x1FF << 30)) >> 30;
    index.PDP = (virtualAddress & ((uint64_t)0x1FF << 39)) >> 39;
    index.PML4 = (virtualAddress & ((uint64_t)0x1FF << 48)) >> 48;
    return index;
}

// misc
void vmmBenchmark();
void vmmInit();
vmm_page_table_t *vmmCreateTable(bool full);

// swap the page table
ifunc void vmmSwap(void *newTable)
{
    controlLoadCR3((uint64_t)newTable); // cr3 is the register that holds the table
}

// mapping
void *vmmAllocateInitialisationVirtualAddress();
void *vmmAllocateInitialisationVirtualAddressPage();
void *vmmMapKernel(void *virtualAddress, void *physicalAddress, uint64_t flags);
void *vmmMap(vmm_page_table_t *table, void *virtualAddress, void *physicalAddress, uint64_t flags);
void vmmUnmap(vmm_page_table_t *table, void *virtualAddress);
void *vmmGetBaseTable();
void *vmmGetPhysical(vmm_page_table_t *table, void *virtualAddress);
void vmmDestroy(vmm_page_table_t *table);