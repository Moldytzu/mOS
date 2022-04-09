#include <idt.h>
#include <pmm.h>
#include <vmm.h>
#include <gdt.h>
#include <serial.h>

struct idt_descriptor idtr;
char *idtData;

void idtSetGate(void *handler, uint8_t entry, uint8_t attributes, bool user)
{
    struct idt_gate_descriptor *gate = (struct idt_gate_descriptor *)(idtr.offset + entry * sizeof(struct idt_gate_descriptor)); // select the gate
    if (gate->segmentselector == 0)                                                                                              // detect if we didn't touch the gate
        idtr.size += sizeof(struct idt_gate_descriptor);                                                                         // if we didn't we can safely increase the size

    gate->attributes = attributes;                         // attributes
    gate->segmentselector = 8;                             // gdt selector
    gate->offset = (uint16_t)((uint64_t)handler) & 0xFFFF; // offsets
    gate->offset2 = (uint16_t)(((uint64_t)handler >> 16)) & 0xFFFF;
    gate->offset3 = (uint32_t)(((uint64_t)handler >> 32));

#ifdef K_IDT_IST
    if (user)
        gate->ist = 2; // set ist if we want user intrerrupt
    else
        gate->ist = 1;
#endif
}

extern void BaseHandlerEntry();
extern void BaseHandler(struct idt_intrerrupt_stack *stack)
{
    printk("Intrerrupt!\n");
    hang();
}

void idtInit()
{
    cli(); // disable intrerrupts

#ifdef K_IDT_IST
    // setup ist
    tssGet()->ist[0] = (uint64_t)mmAllocatePage() + VMM_PAGE;
    tssGet()->ist[1] = (uint64_t)mmAllocatePage() + VMM_PAGE;

    memset64((void*)tssGet()->ist[0] - VMM_PAGE,0,VMM_PAGE/sizeof(uint64_t));
    memset64((void*)tssGet()->ist[1] - VMM_PAGE,0,VMM_PAGE/sizeof(uint64_t));
#endif

    // allocate the idt
    idtData = mmAllocatePage();
    memset64(idtData, 0, VMM_PAGE);

    idtr.offset = (uint64_t)idtData; // set the offset to the data
    idtr.size = 0;                   // reset the size
    for (int i = 0; i < 0xFF; i++)   // set all 255 irqs to the base handler
        idtSetGate((void *)BaseHandlerEntry, i, IDT_InterruptGate, false);

    idtr.size--; // decrement to comply with the spec
    iasm("lidt %0" ::"m"(idtr));
    sti(); // enable intrerrupts
}