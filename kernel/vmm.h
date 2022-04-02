#pragma once
#include <utils.h>

#define VMM_ENTRY_PRESENT 0
#define VMM_ENTRY_RW 1
#define VMM_ENTRY_USER 2
#define VMM_ENTRY_WRITE_THROUGH 3
#define VMM_ENTRY_CACHE 4
#define VMM_ENTRY_ACCESSED 5
#define VMM_ENTRY_DIRTY 6
#define VMM_ENTRY_HUGE_PAGES 7
#define VMM_ENTRY_NO_EXECUTE 63

#define VMM_HHDM 0xFFFF800000000000

#define VMM_PAGE 4096

struct pack vmm_index
{
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
bool vmmGetFlag(uint64_t *entry, uint8_t flag);
void vmmSetFlag(uint64_t *entry, uint8_t flag, bool value);
uint64_t vmmGetAddress(uint64_t *entry);
void vmmSetAddress(uint64_t *entry, uint64_t address);

// indexer
struct vmm_index vmmIndex(uint64_t virtualAddress);

// misc
void vmmInit();
void vmmSwap(void *newTable);
struct pack vmm_page_table *vmmCreateTable(bool hhdm);

// mapping
void vmmMap(struct vmm_page_table *table, void *virtualAddress, void *physicalAddress, bool user, bool rw);
void vmmUnmap(struct vmm_page_table *table, void *virtualAddress);
void *vmmGetBaseTable();
void *vmmGetPhys(struct vmm_page_table *table, void *virtualAddress);