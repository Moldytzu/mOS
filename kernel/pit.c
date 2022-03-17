#include <pit.h>
#include <framebuffer.h>

extern void PITHandlerEntry();
extern void PITHandler()
{
    framebufferWrite("Tick! ");
    picEOI();
}

void pitInit()
{
    asm volatile("cli"); // disable intrerrupts

    // set idt gate
    idtSetGate((void*)PITHandlerEntry,PIC_IRQ_0,IDT_InterruptGate);

    // unmask IRQ 0 on PIC
    outb(PIC_MASTER_DAT,inb(PIC_MASTER_DAT) & ~0b00000001);
    
    asm volatile("sti"); // enable intrerrupts
}