#pragma once
#include <utils.h>

struct pack gdt_descriptor
{
    uint16_t size;
    uint64_t offset;
};

struct pack gdt_segment
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    unsigned limit2 : 4;
    unsigned flags : 4;
    uint8_t base3;
};

struct pack gdt_system_segment
{
    uint16_t limit;
    uint16_t base;
    uint8_t base2;
    uint8_t access;
    uint8_t limit2;
    uint8_t base3;
    uint32_t base4;
    uint32_t ignore;
};

struct pack gdt_tss
{
    uint32_t ignore;
    uint64_t rsp[3]; // stack pointers
    uint64_t ignore2;
    uint64_t ist[7]; // intrerrupt stack table
    uint64_t ignore3;
    uint16_t ignore4;
    uint16_t iopb; // io map base address
};

void gdtCreateSegment(uint8_t access);
void gdtInstallTSS(uint64_t base, uint8_t access);
void gdtInit();
struct gdt_tss *tssGet();