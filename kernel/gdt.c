#include <gdt.h>

struct gdt_tss tss;
struct gdt_descriptor gdtr;
char gdtData[0x1000];

extern void gdtLoad(struct gdt_descriptor *);
extern void tssLoad();

void gdtInit()
{
    gdtr.size = 4 * sizeof(struct gdt_segment) + 1 * sizeof(struct gdt_system_segment) - 1; // 4 segments + 1 system segmnet
    gdtr.offset = (uint64_t)&gdtData[0];

    gdtCreateSegment(0,0); // null
    gdtCreateSegment(0b10011010,8); // kernel code
    gdtCreateSegment(0b10010010,16); // kernel data
    gdtCreateSegment(0b11111010,24); // user code
    gdtCreateSegment(0b11110010,32); // user data

    tss.iopb = sizeof(struct gdt_system_segment);
    memset(&tss,0,sizeof(struct gdt_system_segment)); // clear tss

    gdtCreateSystemSegment((uint64_t)&tss,0b10001001,40); // tss

    gdtLoad(&gdtr); // load gdt and flush segemts
    tssLoad(); // load tss
}

void gdtCreateSegment(uint8_t access, uint8_t offset)
{
    struct gdt_segment *segment = (struct gdt_segment *)(gdtr.offset + offset);
    segment->access = access;
    segment->flags = 0b1010; // 4k pages, long mode
}

void gdtCreateSystemSegment(uint64_t base, uint8_t access, uint8_t offset)
{
    struct gdt_system_segment *segment = (struct gdt_system_segment *)(gdtr.offset + offset);
    segment->access = access;
    segment->flags = 0b1010; // 4k pages, long mode
    segment->base = base & 0x000000000000FFFF;
    segment->base2 = (base & 0x0000000000FF0000) >> 16;
    segment->base3 = (base & 0x00000000FF000000) >> 24;
    segment->base3 = (base & 0xFFFFFFFF00000000) >> 32;
}