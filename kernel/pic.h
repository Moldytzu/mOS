#pragma once
#include <utils.h>
#include <io.h>

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DAT 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DAT 0xA1

#define PIC_EOI_CMD 0x20
#define PIC_INIT_CMD 0x11

void picInit();
void picEOI();