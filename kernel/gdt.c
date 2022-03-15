#include <gdt.h>

struct GDTDescriptor gdtr;
char gdtData[0x1000];

extern void gdtLoad(struct GDTDescriptor *);

void gdtInit()
{
    gdtr.size = 3 * sizeof(struct GDTSegment) - 1; // 3 segments
    gdtr.offset = (uint64_t)&gdtData[0];

    gdtCreateSegment(0,0); // null
    gdtCreateSegment(0b10011010,8); // kernel code
    gdtCreateSegment(0b10010010,16); // kernel data

    gdtLoad(&gdtr);
}

void gdtCreateSegment(uint8_t access, uint8_t offset)
{
    struct GDTSegment *segment = (struct GDTSegment*)(gdtr.offset + offset);
    segment->access = access;
    segment->flags = 0b1010; // 4k pages, long mode
}