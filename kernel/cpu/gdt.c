#include <cpu/gdt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

struct gdt_tss *tss;
struct gdt_descriptor gdtr;
struct gdt_segment *entries;

extern void gdtLoad(struct gdt_descriptor *);
extern void tssLoad();

// initialize the global descriptor table
void gdtInit()
{
    // allocate the entries
    entries = mmAllocatePage();
    memset64(entries, 0, VMM_PAGE / sizeof(uint64_t));

    gdtr.size = 0; // reset the size
    gdtr.offset = (uint64_t)entries;

    gdtCreateSegment(0);          // null
    gdtCreateSegment(0b10011010); // kernel code
    gdtCreateSegment(0b10010010); // kernel data
    gdtCreateSegment(0b11110010); // user data
    gdtCreateSegment(0b11111010); // user code

    tss = mmAllocatePage();                                      // allocate tss
    memset64(tss, 0, sizeof(struct gdt_tss) / sizeof(uint64_t)); // clear it
    gdtInstallTSS((uint64_t)tss, 0b10001001);                    // install it

    gdtr.size--;    // decrement size
    gdtLoad(&gdtr); // load gdt and flush segments
    tssLoad();      // load tss
}

// create a new segment in the table
void gdtCreateSegment(uint8_t access)
{
    struct gdt_segment *segment = &entries[gdtr.size / sizeof(struct gdt_segment)]; // get address of the next segment
    memset64((void *)segment, 0, sizeof(struct gdt_segment) / sizeof(uint64_t));    // clear it
    segment->access = access;                                                       // set the access byte
    segment->flags = 0b1010;                                                        // 4k pages, long mode

    gdtr.size += sizeof(struct gdt_segment); // add the size of gdt_segment
}

// create a new segment and install the tss on it
void gdtInstallTSS(uint64_t base, uint8_t access)
{
    struct gdt_system_segment *segment = (struct gdt_system_segment *)&entries[gdtr.size / sizeof(struct gdt_segment)]; // get address of the next segment
    memset64((void *)segment, 0, sizeof(struct gdt_system_segment) / sizeof(uint64_t));                                 // clear it
    segment->access = access;                                                                                           // set the access byte
    segment->base = base & 0x000000000000FFFF;                                                                          // set the base address of the tss
    segment->base2 = (base & 0x0000000000FF0000) >> 16;
    segment->base3 = (base & 0x00000000FF000000) >> 24;
    segment->base3 = (base & 0xFFFFFFFF00000000) >> 32;
    segment->limit = sizeof(struct gdt_tss) & 0xFFFF; // set the limit of the tss
    segment->limit2 = (sizeof(struct gdt_tss) & 0xF000) >> 16;

    gdtr.size += sizeof(struct gdt_system_segment); // add the size of gdt_system_segment
}

// get the tss address
struct gdt_tss *tssGet()
{
    return tss;
}

// get the gdt segments
struct gdt_segment *gdtGet()
{
    return entries;
}