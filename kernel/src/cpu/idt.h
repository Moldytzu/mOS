#pragma once
#include <misc/utils.h>
#include <drv/framebuffer.h>

pstruct
{
    uint16_t size;
    uint64_t offset;
}
idt_descriptor_t;

pstruct
{
    uint16_t offset;
    uint16_t segmentselector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
}
idt_gate_descriptor_t;

pstruct
{
    uint64_t cr3;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
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
    uint64_t error;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
}
idt_intrerrupt_stack_t;

uint8_t idtAllocateVector();
void idtFreeVector(uint8_t vector);
void idtSetGate(void *handler, uint8_t entry);
void idtInit(uint16_t procID);
void idtInstall(uint8_t procID);
void idtRedirect(void *handler, uint8_t entry, uint32_t tid);
void idtClearRedirect(uint32_t tid);
void *idtGet();