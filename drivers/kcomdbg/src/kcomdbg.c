#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define COM1 0x3F8
#define COM1_LINE_STATUS (COM1 + 5)

drv_type_input_t *input;
char kbuffer[256];
uint8_t kindex = 0;

uint8_t comRead()
{
    while (!(inb(COM1_LINE_STATUS) & 1)) // wait for data ready bit to be set
        sys_yield();

    uint8_t c = inb(COM1);

    if (c == '\r') // convert to our line ending
        c = '\n';

    return c;
}

void comWrite(uint8_t c)
{
    if (c == '\n')
        comWrite('\r');

    while (!(inb(COM1_LINE_STATUS) & 0b100000)) // wait for transmitter buffer empty bit to be set
        sys_yield();

    outb(COM1, c);
}

void comWrites(const char *str)
{
    for (int i = 0; i < strlen(str); i++)
        comWrite(str[i]);
}

void handleInput()
{
    // handle input
    uint8_t lastChar = 0;
    do
    {
        lastChar = comRead();
        comWrite(lastChar);

        if (lastChar != '\n')
            kbuffer[kindex++] = lastChar;
    } while (lastChar != '\n' && kindex != 255);
}

void handleCommands()
{
    if (strcmp(kbuffer, "enter") == 0)
    {
        input->keys[0] = '\n';
    }
    else if (strcmp(kbuffer, "help") == 0)
    {
        comWrites("mOS kernel debugger help\n");
        comWrites("enter - send enter keystroke\n");
    }
}

void _mdrvmain()
{
    input = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT);

    comWrites("\nmOS serial kernel debugger initialised.\n");
    comWrites("Type help for more information\n");

    while (1)
    {
        memset(kbuffer, 0, sizeof(kbuffer));
        kindex = 0;

        comWrites("mOS> ");

        handleInput();
        handleCommands();
    }
}