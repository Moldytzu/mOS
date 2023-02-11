#pragma once
#include <misc/utils.h>
#include <cpu/io.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

#define COM_BAUD_BASE 115200

// write character on the serial console
ifunc void serialWritec(char c)
{
#ifdef K_COM_ENABLE
    while (!(inb(COM1 + 5) & 0b100000))
        ;

    outb(COM1, c); // output the character on the serial console
#endif
}

void serialInit();
void serialWrite(const char *str);