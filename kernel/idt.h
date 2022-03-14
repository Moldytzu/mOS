#pragma once
#include <utils.h>
#include <framebuffer.h>

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

struct pack IDTInterruptStack
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t krsp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void IDTSetGate(struct IDTDescriptor *desc, void *handler, uint8_t entry, uint8_t attributes, uint8_t selector);
void IDTInit();