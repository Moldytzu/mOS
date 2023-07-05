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

void sendKeystroke(char c)
{
    input->keys[0] = c;
    sys_drv_flush(SYS_DRIVER_TYPE_INPUT);
    sys_yield();
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

void strrev(char *str)
{
    size_t len = strlen(str);
    for (int i = 0, j = len - 1; i < j; i++, j--)
    {
        const char a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

// convert to a string (base 16) (taken directly from kernel)
char to_hstringout[32];
const char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    memset(to_hstringout, 0, sizeof(to_hstringout)); // clear output

    for (int i = 0; i < 16; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];    // get each nibble

    strrev(to_hstringout); // reverse string

    // move the pointer until the first valid digit
    uint8_t offset = 0;
    for (; to_hstringout[offset] == '0'; offset++)
        ;

    return to_hstringout + offset; // return the string
}

uint64_t strtoull(const char *input)
{
    uint64_t output = 0;
    // very basic implementation
    for (int i = 0; input[i]; i++)
    {
        char c = input[i];

        if (c >= '0' && c <= '9') // dec 0-9
        {
            output <<= 4;
            output |= c - '0';
        }
        else if (c >= 'A' && c <= 'F') // dec 10-15
        {
            output <<= 4;
            output |= c - 'A' + 10;
        }
    }
    return output;
}

void handleCommands()
{
    // full single-word commands
    if (strcmp(kbuffer, "help") == 0)
    {
        comWrites("mOS kernel debugger help\n");
        comWrites("e - send enter keystroke\n");
        comWrites("w<text> - send text as keystrokes\n");
        comWrites("p<address> - read 512 bytes from address\n");
        return;
    }

    // prefix commands
    switch (kbuffer[0])
    {
    case 'w':
    {
        const char *str = &kbuffer[1]; // text to write

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

    case 'p':
    {
        char *addrStr = &kbuffer[1];
        uint64_t address = strtoull(addrStr);

        // display as hex
        uint8_t *buffer = (uint8_t *)address;
        for (int i = 0; i < 512; i++)
            comWrites(to_hstring(buffer[i]));

        return;
    }

    default:
        comWrites("Unknown command. Use `help` to see all available commands\n");
        break;
    }
}

void _mdrvmain()
{
    input = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT);

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
        memset(kbuffer, 0, sizeof(kbuffer));
        kindex = 0;

        comWrites("mOS> ");

        handleInput();
        handleCommands();
    }
}