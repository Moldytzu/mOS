#include <idt.h>

struct IDTDescriptor idtr;
char idtData[0x1000];

void IDTSetGate(struct IDTDescriptor *desc, void *handler, uint8_t entry, uint8_t attributes, uint8_t selector)
{
    struct IDTGateDescriptor *gate = (struct IDTGateDescriptor *)(desc->offset + entry * sizeof(struct IDTGateDescriptor)); // select the entry
    gate->attributes = attributes; // attributes
    gate->segmentselector = selector; // selector
    gate->offset = (uint16_t)((uint64_t)handler) & 0xFFFF; // offsets
    gate->offset2 = (uint16_t)(((uint64_t)handler >> 16)) & 0xFFFF;
    gate->offset3 = (uint32_t)(((uint64_t)handler >> 32));
}

struct frame;

__attribute__((interrupt)) void IDTBaseHandler(struct frame *frame)
{
    bootloaderTermWrite("Intrerrupt!\n"); // show a message on intrerrupt
    hang();
}

void IDTInit()
{
    asm volatile ("cli"); // disable intrerrupts

    idtr.offset = (uint64_t)&idtData[0];
    idtr.size = 0xFF;
    for(int i = 0;i<255;i++)
        IDTSetGate(&idtr, (void *)IDTBaseHandler,i,IDT_InterruptGate,8);

    asm volatile ("lidt %0" :: "m" (idtr));
    asm volatile ("sti"); // enable intrerrupts
}