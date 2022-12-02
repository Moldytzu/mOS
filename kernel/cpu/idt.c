#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/serial.h>

idt_descriptor_t idtr;
idt_gate_descriptor_t *gates;

// change gate information
void idtSetGate(void *handler, uint8_t entry, uint8_t attributes, bool user)
{
    idt_gate_descriptor_t *gate = &gates[entry]; // select the gate
    zero(gate, sizeof(idt_gate_descriptor_t));
    if (gate->segmentselector == 0)                 // detect if we didn't touch the gate
        idtr.size += sizeof(idt_gate_descriptor_t); // if we didn't we can safely increase the size

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
    tssGet()->ist[0] = (uint64_t)pmmPage() + VMM_PAGE;
    tssGet()->ist[1] = (uint64_t)pmmPage() + VMM_PAGE;

    zero((void *)tssGet()->ist[0] - VMM_PAGE, VMM_PAGE);
    zero((void *)tssGet()->ist[1] - VMM_PAGE, VMM_PAGE);
#endif

    // allocate the gates
    gates = pmmPage();
    zero(gates, VMM_PAGE);

    idtr.offset = (uint64_t)gates; // set the offset to the data
    idtr.size = 0;                 // reset the size
    for (int i = 0; i < 0xFF; i++) // set all exception irqs to the base handler
        idtSetGate((void *)BaseHandlerEntry, i, IDT_InterruptGateU, true);

    idtr.size--;                 // decrement to comply with the spec
    iasm("lidt %0" ::"m"(idtr)); // load the idtr
    sti();                       // enable intrerrupts

    printk("idt: loaded size %d\n", idtr.size);
}