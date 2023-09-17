#include <cpu/gdt.h>
#include <cpu/atomic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <misc/logger.h>

gdt_descriptor_t gdtr[K_MAX_CORES];
gdt_tss_t *tsses[K_MAX_CORES];

extern void gdtLoad(gdt_descriptor_t *);
extern void tssLoad();

void gdtCreateSegment(uint16_t procID, uint8_t access, uint8_t index);
void gdtInstallTSS(uint16_t procID);

void gdtInit()
{
    zero(gdtr, sizeof(gdtr));
    zero(tsses, sizeof(tsses));
}

// install a gdt
void gdtInstall(uint16_t procID)
{
    gdtr[procID].entries = vmmMapKernel(vmmAllocateInitialisationVirtualAddress(), pmmPage(), VMM_ENTRY_RW); // allocate the entries in virtual memory
    gdtr[procID].size = 0;                                                                                   // reset the size

    gdtCreateSegment(procID, 0, 0);          // null
    gdtCreateSegment(procID, 0b10011010, 1); // kernel code
    gdtCreateSegment(procID, 0b10010010, 2); // kernel data
    gdtCreateSegment(procID, 0b11110010, 3); // user data
    gdtCreateSegment(procID, 0b11111010, 4); // user code

    gdtInstallTSS(procID); // install a tss

    gdtr[procID].size--; // decrement size

    gdtLoad(&gdtr[procID]); // load gdt and flush segments
    tssLoad();              // load tss
}

// create a new segment in the table
void gdtCreateSegment(uint16_t procID, uint8_t access, uint8_t index)
{
    gdt_segment_t *segment = &gdtr[procID].entries[index]; // get address of the next segment
    segment->access = access;                              // set the access byte
    segment->flags = 0b1010;                               // 4k pages, long mode

    gdtr[procID].size += sizeof(gdt_segment_t); // add the size of gdt_segment
}

// create a new segment and install the tss on it
void gdtInstallTSS(uint16_t procID)
{
    gdtr[procID].tss = vmmMapKernel(vmmAllocateInitialisationVirtualAddress(), pmmPage(), VMM_ENTRY_RW); // allocate tss in virtual memory
    tsses[procID] = gdtr[procID].tss;                                                                    // remember it

    gdt_system_segment_t *segment = (gdt_system_segment_t *)&gdtr[procID].entries[gdtr[procID].size / sizeof(gdt_segment_t)]; // get address of the next segment

    segment->access = 0b10001001; // set the access byte

    uint64_t address = (uint64_t)gdtr[procID].tss;
    segment->base = (uint16_t)address;
    segment->base2 = (uint8_t)(address >> 16);
    segment->base3 = (uint8_t)(address >> 24);
    segment->base4 = (uint32_t)(address >> 32);

    segment->limit = sizeof(gdt_tss_t); // set the limit of the tss

    gdtr[procID].size += sizeof(gdt_system_segment_t); // add the size of gdt_system_segment
}

// get the tss address
gdt_tss_t **tssGet()
{
    return tsses;
}

gdt_descriptor_t *gdtGet()
{
    return gdtr;
}