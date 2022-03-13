#pragma once
#include <utils.h>
#include <serial.h>

// attributes
#define IDT_InterruptGate 0b10001110
#define IDT_InterruptGateU 0b11101110
#define IDT_CallGate 0b10001100
#define IDT_CallGateU 0b11101100
#define IDT_TrapGate 0b10001111
#define IDT_TrapGateU 0b11101111

// exceptions
#define IDT_DE 0

struct pack IDTDescriptor
{
    uint16_t size;
    uint64_t offset;
};

struct pack IDTGateDescriptor
{
    uint16_t offset;
    uint16_t segmentselector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
};

void IDTSetGate(struct IDTDescriptor *desc, void *handler, uint8_t entry, uint8_t attributes, uint8_t selector);
void IDTInit();