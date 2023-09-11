#include <ps2.h>

// waits for output buffer to be full
void i8042WaitOutputBuffer()
{
    for (int i = 0; i < PS2_TIMEOUT; i++)
    {
        outb(0x80, 0); // io wait

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
    for (int i = 0; i < PS2_TIMEOUT; i++)
    {
        outb(0x80, 0); // io wait

        if (i8042ReadStatus() & (1 << 1) == 0)
            return;
    }

#ifdef PS2_DEBUG
    printf("ps2: i8042WaitInputBuffer timed out!\n");
#endif
}

// read status byte
uint8_t i8042ReadStatus()
{
    return inb(PS2_STATUS);
}

// read output byte
uint8_t i8042ReadOutput()
{
    return inb(PS2_DATA);
}

// writes one byte to data buffer
void i8042WriteData(uint8_t data)
{
    i8042WaitInputBuffer();
    outb(PS2_DATA, data);
}

// writes command to command buffer
void i8042SendCommand(uint8_t cmd)
{
    i8042WaitInputBuffer();
    outb(PS2_COMMAND, cmd);
}

// writes one byte to first port
void port1Write(uint8_t data)
{
    i8042WriteData(data);
}

// writes one byte to second port
void port2Write(uint8_t data)
{
    i8042SendCommand(PS2_CTRL_WRITE_P2);
    i8042FlushBuffers();
    i8042WriteData(data);
}

// flushes all data from the buffers
void i8042FlushBuffers()
{
    i8042WaitOutputBuffer();
    while (i8042ReadStatus() & (1 << 0) > 0) // while the output full bit is set
    {
        // flush
        uint8_t output = i8042ReadOutput();
#ifdef PS2_DEBUG
        printf("ps2: flushed 0x%x\n", output);
#endif
    }
}

// flush one byte off the output buffer
void i8042FlushOnce()
{
    i8042WaitOutputBuffer();
    i8042ReadOutput();
}