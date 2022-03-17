#include <idt.h>

struct IDTDescriptor idtr;
char idtData[0x1000];

void idtSetGate(void *handler, uint8_t entry, uint8_t attributes)
{
    struct IDTGateDescriptor *gate = (struct IDTGateDescriptor *)(idtr.offset + entry * sizeof(struct IDTGateDescriptor)); // select the entry
    gate->attributes = attributes; // attributes
    gate->segmentselector = 8; // gdt selector
    gate->offset = (uint16_t)((uint64_t)handler) & 0xFFFF; // offsets
    gate->offset2 = (uint16_t)(((uint64_t)handler >> 16)) & 0xFFFF;
    gate->offset3 = (uint32_t)(((uint64_t)handler >> 32));
}

extern void BaseHandlerEntry();
extern void BaseHandler(struct IDTInterruptStack *stack)
{
    framebufferWrite("Intrerrupt!\n");
    hang();
}

void idtInit()
{
    asm volatile ("cli"); // disable intrerrupts

    idtr.offset = (uint64_t)&idtData[0];
    idtr.size = 0xFF*sizeof(struct IDTGateDescriptor)-1;
    for(int i = 0;i<0xFF;i++)
        idtSetGate((void *)BaseHandlerEntry,i,IDT_InterruptGate);

    asm volatile ("lidt %0" :: "m" (idtr));
    asm volatile ("sti"); // enable intrerrupts
}