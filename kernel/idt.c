#include <idt.h>
#include <pmm.h>
#include <vmm.h>
#include <gdt.h>
#include <serial.h>

struct idt_descriptor idtr;
struct idt_gate_descriptor *gates;

// change gate information
void idtSetGate(void *handler, uint8_t entry, uint8_t attributes, bool user)
{
    struct idt_gate_descriptor *gate = &gates[entry];    // select the gate
    if (gate->segmentselector == 0)                      // detect if we didn't touch the gate
        idtr.size += sizeof(struct idt_gate_descriptor); // if we didn't we can safely increase the size

    gate->attributes = attributes;                                     // set the attributes
    gate->segmentselector = (8 * 1);                                   // set the kernel code selector from gdt
    gate->offset = (uint16_t)((uint64_t)handler & 0x000000000000ffff); // offset to the entry
    gate->offset2 = (uint16_t)(((uint64_t)handler & 0x00000000ffff0000) >> 16);
    gate->offset3 = (uint32_t)(((uint64_t)handler & 0xffffffff00000000) >> 32);

#ifdef K_IDT_IST
    // enable ists
    if (user)
        gate->ist = 2; // separate ists
    else
        gate->ist = 1;
#endif
}

extern void BaseHandlerEntry();

// initialize the intrerupt descriptor table
void idtInit()
{
    cli(); // disable intrerrupts

#ifdef K_IDT_IST
    // setup ist
    tssGet()->ist[0] = (uint64_t)mmAllocatePage() + VMM_PAGE;
    tssGet()->ist[1] = (uint64_t)mmAllocatePage() + VMM_PAGE;

    memset64((void *)tssGet()->ist[0] - VMM_PAGE, 0, VMM_PAGE / sizeof(uint64_t));
    memset64((void *)tssGet()->ist[1] - VMM_PAGE, 0, VMM_PAGE / sizeof(uint64_t));
#endif

    // allocate the gates
    gates = mmAllocatePage();
    memset64(gates, 0, VMM_PAGE / sizeof(uint64_t));

    idtr.offset = (uint64_t)gates; // set the offset to the data
    idtr.size = 0;                 // reset the size
    for (int i = 0; i < 0xFF; i++) // set all exception irqs to the base handler
        idtSetGate((void *)BaseHandlerEntry, i, IDT_InterruptGate, false);

    idtr.size--;                 // decrement to comply with the spec
    iasm("lidt %0" ::"m"(idtr)); // load the idtr
    sti();                       // enable intrerrupts
}