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

void picInit();
void picEOI();