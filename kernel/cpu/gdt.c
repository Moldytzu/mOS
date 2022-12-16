#include <cpu/gdt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

gdt_tss_t *tss;
gdt_descriptor_t gdtr;
gdt_segment_t *entries;

extern void gdtLoad(gdt_descriptor_t *);
extern void tssLoad();

// initialize the global descriptor table
void gdtInit()
{
    // allocate the entries
    entries = pmmPage();
    zero(entries, VMM_PAGE);

    gdtr.size = 0; // reset the size
    gdtr.offset = (uint64_t)entries;

    gdtCreateSegment(0);          // null
    gdtCreateSegment(0b10011010); // kernel code
    gdtCreateSegment(0b10010010); // kernel data
    gdtCreateSegment(0b11110010); // user data
    gdtCreateSegment(0b11111010); // user code

    tss = pmmPage();                          // allocate tss
    zero(tss, sizeof(gdt_tss_t));             // clear it
    gdtInstallTSS((uint64_t)tss, 0b10001001); // install it

    gdtr.size--;    // decrement size
    gdtLoad(&gdtr); // load gdt and flush segments
    tssLoad();      // load tss

    printk("gdt: loaded cs 0x%x, ss 0x%x and tss 0x%x\n", 8, 8 * 2, 8 * 5);
}

// create a new segment in the table
void gdtCreateSegment(uint8_t access)
{
    gdt_segment_t *segment = &entries[gdtr.size / sizeof(gdt_segment_t)]; // get address of the next segment
    zero(segment, sizeof(gdt_segment_t));                                 // clear the segment
    segment->access = access;                                             // set the access byte
    segment->flags = 0b1010;                                              // 4k pages, long mode

    gdtr.size += sizeof(gdt_segment_t); // add the size of gdt_segment
}

// create a new segment and install the tss on it
void gdtInstallTSS(uint64_t base, uint8_t access)
{
    gdt_system_segment_t *segment = (gdt_system_segment_t *)&entries[gdtr.size / sizeof(gdt_segment_t)]; // get address of the next segment
    zero(segment, sizeof(gdt_system_segment_t));                                                         // clear the segment
    segment->access = access;                                                                            // set the access byte
    segment->base = base & 0x000000000000FFFF;                                                           // set the base address of the tss
    segment->base2 = (base & 0x0000000000FF0000) >> 16;
    segment->base3 = (base & 0x00000000FF000000) >> 24;
    segment->base3 = (base & 0xFFFFFFFF00000000) >> 32;
    segment->limit = sizeof(gdt_tss_t) & 0xFFFF; // set the limit of the tss
    segment->limit2 = (sizeof(gdt_tss_t) & 0xF000) >> 16;

    gdtr.size += sizeof(gdt_system_segment_t); // add the size of gdt_system_segment
}

// get the tss address
gdt_tss_t *tssGet()
{
    return tss;
}

// get the gdt segments
gdt_segment_t *gdtGet()
{
    return entries;
}