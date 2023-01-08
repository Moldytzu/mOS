#pragma once
#include <misc/utils.h>

pstruct
{
    uint16_t size;
    uint64_t offset;
}
gdt_descriptor_t;

pstruct
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    unsigned limit2 : 4;
    unsigned flags : 4;
    uint8_t base3;
}
gdt_segment_t;

pstruct
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    uint8_t limit2;
    uint8_t base3;
    uint32_t base4;
    uint32_t ignore;
}
gdt_system_segment_t;

pstruct
{
    uint32_t ignore;
    uint64_t rsp[3]; // stack pointers
    uint64_t ignore2;
    uint64_t ist[7]; // intrerrupt stack table
    uint64_t ignore3;
    uint16_t ignore4;
    uint16_t iopb; // io map base address
}
gdt_tss_t;

void gdtCreateSegment(uint8_t access);
void gdtInit();
gdt_segment_t *gdtGet();
gdt_tss_t *tssGet();