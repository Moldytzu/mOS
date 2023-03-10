#include <drv/serial.h>
#include <cpu/atomic.h>
#include <misc/logger.h>

// initialize serial controller
void serialInit()
{
#ifdef K_COM_ENABLE
    outb(COM1 + 1, 0);          // disable intrerrupts
    outb(COM1 + 3, 0b10000000); // enable DLAB

    outb(COM1 + 0, K_COM_BAUD_DIV); // set the divisor (baud = COM_BAUD_BASE / K_COM_BAUD_DIV)
    outb(COM1 + 1, 0);

    outb(COM1 + 3, 0b11); // line control register (8 bit character length, 1 stop bit, disable parity, disable break transmission, disable divisor latch)

    outb(COM1 + 2, 0b111); // fifo control register (enable fifo, clear them, disable dma ,trigger at 1 character)

    logInfo("com1: %d baud", COM_BAUD_BASE / K_COM_BAUD_DIV);
#endif
}

// write string on the serial console
void serialWrite(const char *str)
{
#ifdef K_COM_ENABLE
    if (!str)
        return;

    while (*str) // loop thru every character
        serialWritec(*str++);
#endif
}