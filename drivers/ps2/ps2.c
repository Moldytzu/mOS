#include <sys.h>
#include <stdio.h>
#include <string.h>

#define COM1 0x3F8

typedef struct
{
    uint8_t keys[16];        // key buffers
    uint16_t mouseX, mouseY; // mouse coordonates
} drv_type_input_t;

drv_type_input_t *contextStruct;

void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

void _mdrvmain()
{
    printf("started experimental ps2 driver!\n");

    sys_driver(SYS_DRIVER_ANNOUNCE, SYS_DRIVER_TYPE_INPUT, (uint64_t)&contextStruct); // announce that we are an input-related driver

    // some trickery to simulate some key presses until we implement the actual driver code
    const char *toWrite = "ls\n";

    for (int i = 0; i < strlen(toWrite); i++)
        contextStruct->keys[i] = toWrite[i];

    // flush the context
    sys_driver(SYS_DRIVER_FLUSH, SYS_DRIVER_TYPE_INPUT, 0);

    while (1);
}