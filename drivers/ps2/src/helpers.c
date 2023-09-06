#include <ps2.h>

// waits for output buffer to be full
void i8042WaitOutputBuffer()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS; i++)
    {
        sys_yield();

        if (i8042ReadStatus() & (1 << 0) > 0)
            return;
    }

#ifdef PS2_DEBUG
    printf("ps2: i8042WaitOutputBuffer timed out!\n");
#endif
}

// waits for input buffer to be empty
void i8042WaitInputBuffer()
{
    for (int i = 0; i < PS2_TIMEOUT_YIELDS; i++)
    {
        sys_yield();

        if (i8042ReadStatus() & (1 << 1) == 0)
            return;
    }

#ifdef PS2_DEBUG
    printf("ps2: i8042WaitInputBuffer timed out!\n");
#endif
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
    i8042SendCommand(PS2_CTRL_WRITE_P2);
    i8042FlushBuffers();
    i8042WriteData(data);
}

void i8042FlushBuffers()
{
    i8042WaitOutputBuffer();
    uint8_t output = i8042ReadOutput();
#ifdef PS2_DEBUG
    printf("ps2: flushed 0x%x\n", output);
#endif
}