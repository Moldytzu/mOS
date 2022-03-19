#include <gdt.h>

struct gdt_tss tss;
struct gdt_descriptor gdtr;
char gdtData[0x1000];

extern void gdtLoad(struct gdt_descriptor *);
extern void tssLoad();

void gdtInit()
{
    gdtr.size = 0; // reset the size
    gdtr.offset = (uint64_t)&gdtData[0];

    gdtCreateSegment(0); // null
    gdtCreateSegment(0b10011010); // kernel code
    gdtCreateSegment(0b10010010); // kernel data
    gdtCreateSegment(0b11111010); // user code
    gdtCreateSegment(0b11110010); // user data

    memset(&tss,0,sizeof(struct gdt_tss)); // clear tss

    gdtCreateSystemSegment((uint64_t)&tss,0b10001001); // tss

    gdtr.size--; // decrement size
    gdtLoad(&gdtr); // load gdt and flush segemts
    tssLoad(); // load tss
}

void gdtCreateSegment(uint8_t access)
{
    struct gdt_segment *segment = (struct gdt_segment *)(gdtr.offset + gdtr.size);
    memset((void*)segment,0,sizeof(struct gdt_segment));
    segment->access = access;
    segment->flags = 0b1010; // 4k pages, long mode

    gdtr.size += sizeof(struct gdt_segment); // add the size of gdt_segment
}

void gdtCreateSystemSegment(uint64_t base, uint8_t access)
{
    struct gdt_system_segment *segment = (struct gdt_system_segment *)(gdtr.offset + + gdtr.size);
    memset((void*)segment,0,sizeof(struct gdt_system_segment));
    segment->access = access;
    segment->base = base & 0x000000000000FFFF;
    segment->base2 = (base & 0x0000000000FF0000) >> 16;
    segment->base3 = (base & 0x00000000FF000000) >> 24;
    segment->base3 = (base & 0xFFFFFFFF00000000) >> 32;
    segment->limit = sizeof(struct gdt_tss);

    gdtr.size += sizeof(struct gdt_system_segment); // add the size of gdt_system_segment
}