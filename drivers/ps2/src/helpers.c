#include <ps2.h>

void i8042WaitOutputBuffer()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && i8042ReadStatus() & 0b1; i++)
    {
        sys_yield();
    }
}

void i8042WaitInputBuffer()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && i8042ReadStatus() & 0b10; i++)
    {
        sys_yield();
    }
}

uint8_t i8042ReadStatus()
{
    return inb(PS2_STATUS);
}

uint8_t i8042ReadOutput()
{
    return inb(PS2_DATA);
}

void i8042WriteData(uint8_t data)
{
    i8042WaitInputBuffer();
    outb(PS2_DATA, data);
}

void i8042SendCommand(uint8_t cmd)
{
    i8042WaitInputBuffer();
    outb(PS2_COMMAND, cmd);
}

void port1Write(uint8_t data)
{
    i8042WriteData(data);
}

void port2Write(uint8_t data)
{
    outb(PS2_COMMAND, PS2_CTRL_WRITE_P2);
    i8042WriteData(data);
}

void i8042FlushBuffers()
{
    i8042WaitOutputBuffer();
    i8042ReadOutput();
}