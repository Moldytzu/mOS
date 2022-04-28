#pragma once
#include <utils.h>
#include <io.h>

// ports
#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DAT 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DAT 0xA1

// commands
#define PIC_EOI_CMD 0x20
#define PIC_INIT_CMD 0x11

// irqs
#define PIC_IRQ_BASE 0x20
#define PIC_IRQ_0 (PIC_IRQ_BASE + 0)
#define PIC_IRQ_1 (PIC_IRQ_BASE + 1)
#define PIC_IRQ_2 (PIC_IRQ_BASE + 2)
#define PIC_IRQ_3 (PIC_IRQ_BASE + 3)
#define PIC_IRQ_4 (PIC_IRQ_BASE + 4)
#define PIC_IRQ_5 (PIC_IRQ_BASE + 5)
#define PIC_IRQ_6 (PIC_IRQ_BASE + 6)
#define PIC_IRQ_7 (PIC_IRQ_BASE + 7)
#define PIC_IRQ_8 (PIC_IRQ_BASE + 8)
#define PIC_IRQ_9 (PIC_IRQ_BASE + 9)
#define PIC_IRQ_10 (PIC_IRQ_BASE + 10)
#define PIC_IRQ_11 (PIC_IRQ_BASE + 11)
#define PIC_IRQ_12 (PIC_IRQ_BASE + 12)
#define PIC_IRQ_13 (PIC_IRQ_BASE + 13)

void picInit();

// send end of intrerrupt command to both chips
ifunc void picEOI()
{
    outb(PIC_SLAVE_CMD, PIC_EOI_CMD);
    outb(PIC_MASTER_CMD, PIC_EOI_CMD);
}