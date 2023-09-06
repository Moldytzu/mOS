#include <ps2.h>

void waitOutput()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b1; i++)
    {
        sys_yield();
    }
}

void waitInput()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b10; i++)
    {
        sys_yield();
    }
}

uint8_t status()
{
    return inb(PS2_STATUS);
}

uint8_t output()
{
    return inb(PS2_DATA);
}

void write(uint8_t data)
{
    waitInput();
    outb(PS2_DATA, data);
}

void command(uint8_t cmd)
{
    waitInput();
    outb(PS2_COMMAND, cmd);
}

void port1Write(uint8_t data)
{
    write(data);
}

void port2Write(uint8_t data)
{
    outb(PS2_COMMAND, PS2_CTRL_WRITE_P2);
    write(data);
}

void flush()
{
    waitOutput();
    output();
}