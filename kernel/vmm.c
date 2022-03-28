#include <vmm.h>
#include <pmm.h>

void *baseTable;

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
}