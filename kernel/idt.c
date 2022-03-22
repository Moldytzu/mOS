#include <idt.h>
#include <mm.h>

struct idt_descriptor idtr;
char *idtData;

void idtSetGate(void *handler, uint8_t entry, uint8_t attributes)
{
    struct idt_gate_descriptor *gate = (struct idt_gate_descriptor *)(idtr.offset + entry * sizeof(struct idt_gate_descriptor)); // select the gate
    if (gate->segmentselector == 0)                                                                                              // detect if we didn't touch the gate
        idtr.size += sizeof(struct idt_gate_descriptor);                                                                         // if we didn't we can safely increase the size

    gate->attributes = attributes;                         // attributes
    gate->segmentselector = 8;                             // gdt selector
    gate->offset = (uint16_t)((uint64_t)handler) & 0xFFFF; // offsets
    gate->offset2 = (uint16_t)(((uint64_t)handler >> 16)) & 0xFFFF;
    gate->offset3 = (uint32_t)(((uint64_t)handler >> 32));
}

extern void BaseHandlerEntry();
extern void BaseHandler(struct idt_intrerrupt_stack *stack)
{
    printk("Intrerrupt!\n");
    hang();
}

void idtInit()
{
    iasm("cli"); // disable intrerrupts

    // allocate the idt
    idtData = mmAllocatePage();
    memset64(idtData, 0, 4096);

    idtr.offset = (uint64_t)idtData; // set the offset to the data
    idtr.size = 0;                   // reset the size
    for (int i = 0; i < 0xFF; i++)   // set all 255 irqs to the base handler
        idtSetGate((void *)BaseHandlerEntry, i, IDT_InterruptGate);

    idtr.size--; // decrement to comply with the spec
    iasm("lidt %0" ::"m"(idtr));
    iasm("sti"); // enable intrerrupts
}