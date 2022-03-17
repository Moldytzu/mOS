#include <idt.h>

struct idt_descriptor idtr;
char idtData[0x1000];

void idtSetGate(void *handler, uint8_t entry, uint8_t attributes)
{
    struct idt_gate_descriptor *gate = (struct idt_gate_descriptor *)(idtr.offset + entry * sizeof(struct idt_gate_descriptor)); // select the gate
    gate->attributes = attributes; // attributes
    gate->segmentselector = 8; // gdt selector
    gate->offset = (uint16_t)((uint64_t)handler) & 0xFFFF; // offsets
    gate->offset2 = (uint16_t)(((uint64_t)handler >> 16)) & 0xFFFF;
    gate->offset3 = (uint32_t)(((uint64_t)handler >> 32));
}

extern void BaseHandlerEntry();
extern void BaseHandler(struct idt_intrerrupt_stack *stack)
{
    framebufferWrite("Intrerrupt!\n");
    hang();
}

void idtInit()
{
    asm volatile ("cli"); // disable intrerrupts

    idtr.offset = (uint64_t)&idtData[0]; // set the offset to the data
    idtr.size = 0xFF*sizeof(struct idt_gate_descriptor)-1; // the size is in bytes -1
    for(int i = 0;i<0xFF;i++) // set all 255 irqs to the base handler
        idtSetGate((void *)BaseHandlerEntry,i,IDT_InterruptGate);

    asm volatile ("lidt %0" :: "m" (idtr));
    asm volatile ("sti"); // enable intrerrupts
}