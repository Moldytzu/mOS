#include <cpu/gdt.h>
#include <cpu/atomic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

gdt_descriptor_t gdtr[K_MAX_CORES];

extern void gdtLoad(gdt_descriptor_t *);
extern void tssLoad();

void gdtCreateSegment(uint16_t procID, uint8_t access);
void gdtInstallTSS(uint16_t procID);

// install a gdt
void gdtInstall(uint16_t procID)
{
    // allocate the entries
    gdtr[procID].entries = pmmPage();
    zero(gdtr[procID].entries, VMM_PAGE);

    gdtr[procID].size = 0; // reset the size

    gdtCreateSegment(procID, 0);          // null
    gdtCreateSegment(procID, 0b10011010); // kernel code
    gdtCreateSegment(procID, 0b10010010); // kernel data
    gdtCreateSegment(procID, 0b11110010); // user data
    gdtCreateSegment(procID, 0b11111010); // user code

    gdtInstallTSS(procID); // install a tss

    gdtr[procID].size--; // decrement size

    gdtLoad(&gdtr[procID]); // load gdt and flush segments
    tssLoad();              // load tss
}

// create a new segment in the table
void gdtCreateSegment(uint16_t procID, uint8_t access)
{
    gdt_segment_t *segment = &gdtr[procID].entries[gdtr[procID].size / sizeof(gdt_segment_t)]; // get address of the next segment
    zero(segment, sizeof(gdt_segment_t));                                                      // clear the segment
    segment->access = access;                                                                  // set the access byte
    segment->flags = 0b1010;                                                                   // 4k pages, long mode

    gdtr[procID].size += sizeof(gdt_segment_t); // add the size of gdt_segment
}

// create a new segment and install the tss on it
void gdtInstallTSS(uint16_t procID)
{
    gdtr[procID].tss = pmmPage();              // allocate tss
    zero(gdtr[procID].tss, sizeof(gdt_tss_t)); // clear it

    gdt_system_segment_t *segment = (gdt_system_segment_t *)&gdtr[procID].entries[gdtr[procID].size / sizeof(gdt_segment_t)]; // get address of the next segment
    zero(segment, sizeof(gdt_system_segment_t));                                                                              // clear the segment

    segment->access = 0b10001001; // set the access byte

    segment->base = (uint64_t)gdtr[procID].tss & 0x000000000000FFFF; // set the base address of the tss
    segment->base2 = ((uint64_t)gdtr[procID].tss & 0x0000000000FF0000) >> 16;
    segment->base3 = ((uint64_t)gdtr[procID].tss & 0x00000000FF000000) >> 24;
    segment->base3 = ((uint64_t)gdtr[procID].tss & 0xFFFFFFFF00000000) >> 32;

    segment->limit = sizeof(gdt_tss_t) & 0xFFFF; // set the limit of the tss
    segment->limit2 = (sizeof(gdt_tss_t) & 0xF000) >> 16;

    gdtr[procID].size += sizeof(gdt_system_segment_t); // add the size of gdt_system_segment
}

// get the tss address
gdt_tss_t *tssGet()
{
    return NULL;
}

// get the gdt segments
gdt_segment_t *gdtGet()
{
    return NULL;
}