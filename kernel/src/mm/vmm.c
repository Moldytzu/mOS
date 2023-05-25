#include <mm/vmm.h>
#include <mm/pmm.h>
#include <drv/serial.h>
#include <fw/bootloader.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/smp.h>
#include <cpu/atomic.h>
#include <cpu/lapic.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <sched/hpet.h>

#define PAGE_ADDR(x) ((vmm_page_table_t *)(x & 0xFFFFFFFFFF000))

vmm_page_table_t *baseTable;

// initialize the virtual memory manager
void vmmInit()
{
    baseTable = vmmCreateTable(true); // create the base table with hhdm
    vmmSwap(baseTable);               // swap the table

    logInfo("vmm: loaded a new page table");
}

// set flags of some entries given by the indices
ifunc void vmmSetFlags(vmm_page_table_t *table, vmm_index_t index, uint64_t flags)
{
    table->entries[index.PDP] |= flags; // index pdp

    table = PAGE_ADDR(table->entries[index.PDP]); // index pd
    table->entries[index.PD] |= flags;

    table = PAGE_ADDR(table->entries[index.PD]); // index pt
    table->entries[index.PT] |= flags;

    table = PAGE_ADDR(table->entries[index.PT]); // index the page
    table->entries[index.P] |= flags;
}

// map a virtual address to a physical address in a page table
void vmmMap(vmm_page_table_t *table, void *virtualAddress, void *physicalAddress, uint64_t flags)
{
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pml4, *pdp, *pd, *pt;
    uint64_t currentEntry;

    pml4 = table;

    currentEntry = pml4->entries[index.PDP]; // index pdp
    if (!(currentEntry & VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pdp = pmmPage();                                             // allocate table
        vmmSetAddress(&currentEntry, (uint64_t)pdp >> 12);           // set it's address
        pml4->entries[index.PDP] = currentEntry | VMM_ENTRY_PRESENT; // write the entry in the table
    }
    else
        pdp = PAGE_ADDR(currentEntry); // continue

    currentEntry = pdp->entries[index.PD];   // index pd
    if (!(currentEntry & VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pd = pmmPage();                                            // allocate table
        vmmSetAddress(&currentEntry, (uint64_t)pd >> 12);          // set it's address
        pdp->entries[index.PD] = currentEntry | VMM_ENTRY_PRESENT; // write the entry in the table
    }
    else
        pd = PAGE_ADDR(currentEntry); // continue

    currentEntry = pd->entries[index.PT];    // index pt
    if (!(currentEntry & VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pt = pmmPage();                                           // allocate table
        vmmSetAddress(&currentEntry, (uint64_t)pt >> 12);         // set it's address
        pd->entries[index.PT] = currentEntry | VMM_ENTRY_PRESENT; // write the entry in the table
    }
    else
        pt = PAGE_ADDR(currentEntry); // continue

    currentEntry = pt->entries[index.P];                           // index p
    vmmSetAddress(&currentEntry, (uint64_t)physicalAddress >> 12); // set the address to the physical one
    pt->entries[index.P] = currentEntry | VMM_ENTRY_PRESENT;       // write the entry in the table

    vmmSetFlags(table, index, flags); // set the flags
}

// unmap a virtual address
void vmmUnmap(vmm_page_table_t *table, void *virtualAddress)
{
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    pdp = PAGE_ADDR(currentEntry);

    currentEntry = pdp->entries[index.PD]; // index pd
    pd = PAGE_ADDR(currentEntry);

    currentEntry = pd->entries[index.PT]; // index pt
    pt = PAGE_ADDR(currentEntry);

    currentEntry = pt->entries[index.P];                      // index p
    pt->entries[index.P] = currentEntry & ~VMM_ENTRY_PRESENT; // unset present flag

    tlbFlush(virtualAddress);
}

// get the base table aka kernel table
void *vmmGetBaseTable()
{
    return baseTable;
}

// get physical address of a virtual address
void *vmmGetPhys(vmm_page_table_t *table, void *virtualAddress)
{
    // get physical memory address form virtual memory address
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    pdp = PAGE_ADDR(currentEntry);                     // continue

    currentEntry = pdp->entries[index.PD]; // index pd
    pd = PAGE_ADDR(currentEntry);          // continue

    currentEntry = pd->entries[index.PT]; // index pt
    pt = PAGE_ADDR(currentEntry);         // continue

    currentEntry = pt->entries[index.P]; // index p

    return (void *)(((uint64_t)PAGE_ADDR(currentEntry) >> 12) * VMM_PAGE + ((uint64_t)virtualAddress % VMM_PAGE)); // get the address
}

// create a new table
vmm_page_table_t *vmmCreateTable(bool full)
{
    uint64_t a = pmmTotal().available;

    // create a new table to use as a base for everything
    vmm_page_table_t *newTable = (vmm_page_table_t *)pmmPage();

    struct limine_memmap_response *memMap = bootloaderGetMemoryMap();
    uint64_t hhdm = (uint64_t)bootloaderGetHHDM();
    struct limine_kernel_address_response *kaddr = bootloaderGetKernelAddress();

    // map the system tables as kernel rw
    vmmMap(newTable, newTable, newTable, VMM_ENTRY_RW); // page table

    // map system tables for non-kernel tables (kernel tables have everything mapped)
    if (!full)
    {
        for (int i = 0; i < smpCores(); i++) // map gdt and tss of each core
        {
            gdt_tss_t *tss = tssGet()[i];
            gdt_descriptor_t gdt = gdtGet()[i];

            vmmMap(newTable, (void *)tss, (void *)tss, VMM_ENTRY_RW);                               // tss struct
            vmmMap(newTable, (void *)tss->ist[0] - 4096, (void *)tss->ist[0] - 4096, VMM_ENTRY_RW); // kernel ist
            vmmMap(newTable, (void *)tss->ist[1] - 4096, (void *)tss->ist[1] - 4096, VMM_ENTRY_RW); // userspace ist

            vmmMap(newTable, gdt.entries, gdt.entries, VMM_ENTRY_RW);   // gdt entries
            vmmMap(newTable, &gdtGet()[i], &gdtGet()[i], VMM_ENTRY_RW); // gdtr
        }
    }

    // map memory map entries as kernel rw
    for (size_t i = 0; i < memMap->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memMap->entries[i];

        if (entry->type != LIMINE_MEMMAP_KERNEL_AND_MODULES && !full) // don't map any type of memory besides kernel
            continue;

        if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
        {
            for (size_t i = 0; i < entry->length; i += 4096)
                vmmMap(newTable, (void *)(kaddr->virtual_base + i), (void *)(kaddr->physical_base + i), VMM_ENTRY_RW);
        }

        if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER)
        {
            for (size_t i = 0; i < entry->length; i += 4096)
                vmmMap(newTable, (void *)(kaddr->virtual_base + i), (void *)(kaddr->physical_base + i), VMM_ENTRY_RW | VMM_ENTRY_WRITE_THROUGH);
        }

        for (size_t i = 0; i < entry->length; i += 4096)
        {
            vmmMap(newTable, (void *)(entry->base + i), (void *)(entry->base + i), VMM_ENTRY_USER | VMM_ENTRY_RW);
            vmmMap(newTable, (void *)(entry->base + i + hhdm), (void *)(entry->base + i), VMM_ENTRY_RW);
        }
    }

    vmmMap(newTable, idtGet(), idtGet(), VMM_ENTRY_RW);
    vmmMap(newTable, APIC_BASE, APIC_BASE, VMM_ENTRY_RW | VMM_ENTRY_CACHE_DISABLE);

    logDbg(LOG_SERIAL_ONLY, "vmm: wasted %d KB on a new page table", (a - pmmTotal().available) / 1024);

    return newTable; // return the created table
}

// free a table
void vmmDestroy(vmm_page_table_t *table)
{
#ifdef K_VMM_DEBUG
    uint64_t a = pmmTotal().available;
#endif

    if(!table)
        return;

    // deallocate sub-tables
    for (int pdp = 0; pdp < 512; pdp++)
    {
        uint64_t pdpValue = table->entries[pdp];

        if (!pdpValue)
            continue;

        vmm_page_table_t *pdpPtr = PAGE_ADDR(pdpValue);

        for (int pd = 0; pd < 512; pd++)
        {
            uint64_t pdValue = pdpPtr->entries[pd];

            if (!pdValue)
                continue;

            vmm_page_table_t *pdPtr = PAGE_ADDR(pdValue);

            for (int pt = 0; pt < 512; pt++)
            {
                uint64_t ptValue = pdPtr->entries[pt];

                if (!ptValue)
                    continue;

                vmm_page_table_t *ptPtr = PAGE_ADDR(ptValue);

                pmmDeallocate(ptPtr);
            }

            pmmDeallocate(pdPtr);
        }

        pmmDeallocate(pdpPtr);
    }

    // deallocate main table
    pmmDeallocate(table);

#ifdef K_VMM_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vmm: destroyed page table at 0x%p and saved %d kb", table, (pmmTotal().available - a) / 1024);
#endif
}

#define VMM_BENCHMARK_SIZE 10
void vmmBenchmark()
{
    // test performance of the mapper
    logInfo("vmm: benchmarking");

    uint64_t start = hpetMillis();

    void *addr[VMM_BENCHMARK_SIZE];

    for (int i = 0; i < VMM_BENCHMARK_SIZE; i++) // allocate
        addr[i] = vmmCreateTable(true);

    uint64_t end = hpetMillis();

    logInfo("vmm: it took %d miliseconds to create %d", end - start, VMM_BENCHMARK_SIZE);

    hang();
}