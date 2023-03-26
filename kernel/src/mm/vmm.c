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

// todo: don't use booleans to set flags, instead let the function take an uint64_t and or it over the flags thus changing them easily and makes this extensible if a flag is needed

locker_t vmmLock; // todo: replace this with a per-table lock
vmm_page_table_t *baseTable;

// initialize the virtual memory manager
void vmmInit()
{
    baseTable = vmmCreateTable(true, false); // create the base table with hhdm
    vmmSwap(baseTable);                      // swap the table

    logInfo("vmm: loaded a new page table");
}

// set flags of some entries given by the indices
inline __attribute__((always_inline)) void vmmSetFlags(vmm_page_table_t *table, vmm_index_t index, bool user, bool rw, bool wt, bool cache)
{
    vmm_page_table_t *pml4, *pdp, *pd, *pt;
    uint64_t currentEntry;
    pml4 = table;

    currentEntry = pml4->entries[index.PDP];         // index pdp
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);     // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user); // userspace
    pml4->entries[index.PDP] = currentEntry;         // write the entry in the table

    pdp = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pdp->entries[index.PD];                          // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                    // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);                // userspace
    pdp->entries[index.PD] = currentEntry;                          // write the entry in the table

    pd = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pd->entries[index.PT];                          // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                   // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);               // userspace
    pd->entries[index.PT] = currentEntry;                          // write the entry in the table

    pt = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue
    currentEntry = pt->entries[index.P];                           // index further
    vmmSetFlag(&currentEntry, VMM_ENTRY_RW, rw);                   // read-write
    vmmSetFlag(&currentEntry, VMM_ENTRY_USER, user);               // userspace
    vmmSetFlag(&currentEntry, VMM_ENTRY_WRITE_THROUGH, wt);        // write-through
    vmmSetFlag(&currentEntry, VMM_ENTRY_CACHE_DISABLE, cache);     // cache disable
    pt->entries[index.P] = currentEntry;                           // write the entry in the table
}

// map a virtual address to a physical address in a page table
void vmmMap(vmm_page_table_t *table, void *virtualAddress, void *physicalAddress, bool user, bool rw, bool wt, bool cache)
{
    atomicAquire(&vmmLock);
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pml4, *pdp, *pd, *pt;
    uint64_t currentEntry;

    pml4 = table;

    currentEntry = pml4->entries[index.PDP];           // index pdp
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pdp = pmmPage();     // allocate table
        zero(pdp, VMM_PAGE); // clear it

        vmmSetAddress(&currentEntry, (uint64_t)pdp >> 12);  // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pml4->entries[index.PDP] = currentEntry;            // write the entry in the table
    }
    else
        pdp = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pdp->entries[index.PD];             // index pd
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pd = pmmPage();     // allocate table
        zero(pd, VMM_PAGE); // clear it

        vmmSetAddress(&currentEntry, (uint64_t)pd >> 12);   // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pdp->entries[index.PD] = currentEntry;              // write the entry in the table
    }
    else
        pd = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pd->entries[index.PT];              // index pt
    if (!vmmGetFlag(&currentEntry, VMM_ENTRY_PRESENT)) // if there isn't any page present there, we generate it
    {
        pt = pmmPage();     // allocate table
        zero(pt, VMM_PAGE); // clear it

        vmmSetAddress(&currentEntry, (uint64_t)pt >> 12);   // set it's address
        vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true); // present
        pd->entries[index.PT] = currentEntry;               // write the entry in the table
    }
    else
        pt = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pt->entries[index.P];                           // index p
    vmmSetAddress(&currentEntry, (uint64_t)physicalAddress >> 12); // set the address to the physical one
    vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, true);            // present
    pt->entries[index.P] = currentEntry;                           // write the entry in the table

    vmmSetFlags(table, index, user, rw, wt, cache); // set the flags
    tlbFlush(virtualAddress);
    atomicRelease(&vmmLock);
}

// unmap a virtual address
void vmmUnmap(vmm_page_table_t *table, void *virtualAddress)
{
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP]; // index pdp
    pdp = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pdp->entries[index.PD]; // index pd
    pd = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pd->entries[index.PT]; // index pt
    pt = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12);

    currentEntry = pt->entries[index.P];                 // index p
    vmmSetFlag(&currentEntry, VMM_ENTRY_PRESENT, false); // unvalidate page
    pt->entries[index.P] = currentEntry;                 // write the entry in the table

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
    tlbFlush(virtualAddress);

    // get physical memory address form virtual memory address
    vmm_index_t index = vmmIndex((uint64_t)virtualAddress); // get the offsets in the page tables
    vmm_page_table_t *pdp, *pd, *pt;

    uint64_t currentEntry = table->entries[index.PDP];              // index pdp
    pdp = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pdp->entries[index.PD];                         // index pd
    pd = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pd->entries[index.PT];                          // index pt
    pt = (vmm_page_table_t *)(vmmGetAddress(&currentEntry) << 12); // continue

    currentEntry = pt->entries[index.P]; // index p

    return (void *)(vmmGetAddress(&currentEntry) * VMM_PAGE + ((uint64_t)virtualAddress % VMM_PAGE)); // get the address
}

// create a new table
vmm_page_table_t *vmmCreateTable(bool full, bool driver)
{
    uint64_t a = pmmTotal().available;

    // create a new table to use as a base for everything
    vmm_page_table_t *newTable = (vmm_page_table_t *)pmmPages(2);

    zero(newTable, VMM_PAGE + 16); // zero out the table and the metadata

    struct limine_memmap_response *memMap = bootloaderGetMemoryMap();
    uint64_t hhdm = (uint64_t)bootloaderGetHHDM();
    struct limine_kernel_address_response *kaddr = bootloaderGetKernelAddress();

    // map the system tables as kernel rw
    vmmMap(newTable, newTable, newTable, driver, true, false, false); // page table

    // map system tables for non-kernel tables (kernel tables have everything mapped)
    if (!full)
    {
        for (int i = 0; i < smpCores(); i++) // map gdt and tss of each core
        {
            gdt_tss_t *tss = tssGet()[i];
            gdt_descriptor_t gdt = gdtGet()[i];

            vmmMap(newTable, (void *)tss, (void *)tss, false, true, false, false);                               // tss struct
            vmmMap(newTable, (void *)tss->ist[0] - 4096, (void *)tss->ist[0] - 4096, false, true, false, false); // kernel ist
            vmmMap(newTable, (void *)tss->ist[1] - 4096, (void *)tss->ist[1] - 4096, false, true, false, false); // userspace ist

            vmmMap(newTable, gdt.entries, gdt.entries, false, true, false, false);   // gdt entries
            vmmMap(newTable, &gdtGet()[i], &gdtGet()[i], false, true, false, false); // gdtr
        }

        vmmMap(newTable, idtGet(), idtGet(), false, true, false, false);
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
                vmmMap(newTable, (void *)(kaddr->virtual_base + i), (void *)(kaddr->physical_base + i), driver, true, false, false);
        }
        for (size_t i = 0; i < entry->length; i += 4096)
        {
            vmmMap(newTable, (void *)(entry->base + i), (void *)(entry->base + i), driver, true, false, false);
            vmmMap(newTable, (void *)(entry->base + i + hhdm), (void *)(entry->base + i), driver, true, false, false);
        }
    }

    // map lapic
    vmmMap(newTable, lapicBase(), lapicBase(), false, true, false, false);

    logDbg(LOG_ALWAYS, "vmm: wasted %d KB on a new page table", toKB(a - pmmTotal().available));

    return newTable; // return the created table
}

// free a table
void vmmDestroy(vmm_page_table_t *table)
{
#ifdef K_VMM_DEBUG
    uint64_t a = pmmTotal().available;
#endif

    lock(vmmLock, {
        // deallocate sub-tables
        for (int pdp = 0; pdp < 512; pdp++)
        {
            uint64_t pdpValue = table->entries[pdp];

            if (!pdpValue)
                continue;

            vmm_page_table_t *pdpPtr = (void *)(vmmGetAddress(&pdpValue) << 12);

            for (int pd = 0; pd < 512; pd++)
            {
                uint64_t pdValue = pdpPtr->entries[pd];

                if (!pdValue)
                    continue;

                vmm_page_table_t *pdPtr = (void *)(vmmGetAddress(&pdValue) << 12);

                for (int pt = 0; pt < 512; pt++)
                {
                    uint64_t ptValue = pdPtr->entries[pt];

                    if (!ptValue)
                        continue;

                    vmm_page_table_t *ptPtr = (void *)(vmmGetAddress(&ptValue) << 12);

                    pmmDeallocate(ptPtr);
                }

                pmmDeallocate(pdPtr);
            }

            pmmDeallocate(pdpPtr);
        }

        // deallocate main table
        pmmDeallocatePages(table, 2);
    });

#ifdef K_VMM_DEBUG
    printks("vmm: destroyed page table at 0x%p and saved %d kb\n\r", table, toKB(pmmTotal().available - a));
#endif
}

#define VMM_BENCHMARK_SIZE 70
void vmmBenchmark()
{
    // test performance of the mapper
    logInfo("vmm: benchmarking");

    uint64_t start = hpetMillis();

    void *addr[VMM_BENCHMARK_SIZE];

    for (int i = 0; i < VMM_BENCHMARK_SIZE; i++) // allocate
        addr[i] = vmmCreateTable(true, false);

    uint64_t end = hpetMillis();

    logInfo("vmm: it took %d miliseconds", end - start);

    hang();
}