#include <vmm.h>
#include <pmm.h>

void *baseTable;

void vmmInit()
{
    void *bootloaderTable = NULL;
    iasm ("mov %%cr3, %%rax" : "=a"(bootloaderTable)); // get bootloader's paging table

    baseTable = mmAllocatePage(); // allocate a page for the base table
    memcpy64(baseTable,bootloaderTable,4096/sizeof(uint64_t)); // copy bootloader's paging table over our base table

    iasm ("mov %0, %%cr3" :: "r"(baseTable)); // set cr3 to our "new" table
}