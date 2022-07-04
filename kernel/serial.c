#include <serial.h>

// initialize serial controller
void serialInit()
{
    outb(COM1 + 1, 0);          // disable intrerrupts
    outb(COM1 + 3, 0b10000000); // enable DLAB

    outb(COM1 + 0, 1); // divisor 1 (baud 115200 / 1)
    outb(COM1 + 1, 0);

    outb(COM1 + 3, 0b11); // line control register (8 bit character length, 1 stop bit, disable parity, disable break transmission, disable divisor latch)

    outb(COM1 + 2, 0b111); // fifo control register (enable fifo, clear them, disable dma ,trigger at 1 character)
}

// write string on the serial console
void serialWrite(const char *str)
{
    while (*str) // loop thru every character
    {
        if (*str == '\n')
            serialWritec('\r'); // the serial console uses the CRLF end line method and we don't
        serialWritec(*str);
        str++;
    }
}