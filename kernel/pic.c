#include <pic.h>

void picInit()
{
    // start initialization sequence
    outb(PIC_MASTER_CMD,PIC_INIT_CMD);
    outb(PIC_SLAVE_CMD,PIC_INIT_CMD);

    // 0x20 offset
    outb(PIC_MASTER_DAT, 0x20); 
    outb(PIC_SLAVE_DAT, 0x20); 

    // inform the chips that they're working together
    outb(PIC_MASTER_DAT, 4);
    outb(PIC_SLAVE_DAT, 2);

    // set operating mode to i8086
    outb(PIC_MASTER_DAT, 1);
    outb(PIC_SLAVE_DAT, 1);

    // mask all the intrerrupts
    outb(PIC_MASTER_DAT, 0b11111111);
    outb(PIC_SLAVE_DAT, 0b11111111);
}

void picEOI()
{
    // send end of intrerrupt command to both chips
    outb(PIC_SLAVE_CMD,PIC_EOI_CMD);
    outb(PIC_MASTER_CMD,PIC_EOI_CMD);
}