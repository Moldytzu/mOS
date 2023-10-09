#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define COM1 0x3F8
#define COM1_LINE_STATUS (COM1 + 5)

drv_type_input_t *input;
char kbuffer[256];
uint8_t kindex = 0;

const volatile drv_metadata_section_t metadata __attribute__((section(".mdrivermeta"))) = {};

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

void sendKeystroke(char c)
{
    input->keys[0] = c;
    sys_driver_flush(SYS_DRIVER_TYPE_INPUT);
    sys_yield();
}

void handleInput()
{
    // handle input
    while (true)
    {
        uint8_t receivedCharacter = comRead(); // read character (fixme: this may be scancode on real hardware??)
        comWrite(receivedCharacter);           // loop back received character (fixme: on real hardware this is garbage??)

        if (receivedCharacter == '\n' || kindex == 256) // break if we receive a line feed (the enter key) or if we overrun the buffer
            break;

        kbuffer[kindex++] = receivedCharacter; // put the character on our buffer
    }
}

void handleCommands()
{
    // full single-word commands
    if (strcmp(kbuffer, "help") == 0)
    {
        comWrites("mOS kernel debugger help\n");
        comWrites("e - send enter keystroke\n");
        comWrites("w<text> - send text as keystrokes\n");
        comWrites("r - reboots the computer\n");
        comWrites("s - shutdowns the computer\n");
        return;
    }

    // prefix commands
    switch (kbuffer[0])
    {
    case 'w':
    {
        const char *str = &kbuffer[1]; // text to write (skips the w)

        for (int i = 0; i < strlen(str); i++)
        {
            comWrite(str[i]);
            sendKeystroke(str[i]);
        }
        return;
    }

    case 'e':
    {
        sendKeystroke('\n');
        return;
    }

    case 'r':
    {
        sys_power(SYS_POWER_REBOOT, 0, 0);
        return;
    }

    case 's':
    {
        sys_power(SYS_POWER_SHUTDOWN, 0, 0);
        return;
    }

    default:
        comWrites("Unknown command. Use `help` to see all available commands\n");
        break;
    }
}

void _mdrvmain()
{
    input = (drv_type_input_t *)sys_driver_announce(SYS_DRIVER_TYPE_INPUT);

    if (!input)
    {
        comWrites("kcomdbg: failed to announce!\n");
        abort();
    }

    comWrites("\nmOS serial kernel debugger initialised.\n");
    comWrites("Type help for more information\n");

    puts("kcomdbg: launched debug console on serial COM1\n");

    while (1)
    {
        // reset keyboard buffer
        memset(kbuffer, 0, sizeof(kbuffer));
        kindex = 0;

        // write prompt
        comWrites("mOS> ");

        // handle user-input
        handleInput();
        handleCommands();
    }
}