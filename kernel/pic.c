#include <pic.h>

void picInit()
{
    cli(); // disable intrerrupts

    // start initialization sequence
    outb(PIC_MASTER_CMD, PIC_INIT_CMD);
    outb(PIC_SLAVE_CMD, PIC_INIT_CMD);

    // set the offset/base
    outb(PIC_MASTER_DAT, PIC_IRQ_BASE);
    outb(PIC_SLAVE_DAT, PIC_IRQ_BASE);

    // inform the chips that they're working together
    outb(PIC_MASTER_DAT, 4);
    outb(PIC_SLAVE_DAT, 2);

    // set operating mode to i8086
    outb(PIC_MASTER_DAT, 1);
    outb(PIC_SLAVE_DAT, 1);

    // mask all the intrerrupts
    outb(PIC_MASTER_DAT, 0b11111111);
    outb(PIC_SLAVE_DAT, 0b11111111);

#ifndef K_IDT_DIS_KIRQ
    sti(); // enable intrerrupts
#endif
}

void picEOI()
{
    // send end of intrerrupt command to both chips
    outb(PIC_SLAVE_CMD, PIC_EOI_CMD);
    outb(PIC_MASTER_CMD, PIC_EOI_CMD);
}