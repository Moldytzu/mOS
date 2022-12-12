#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <string.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

#define PS2_TYPE_INVALID 0xFF
#define PS2_TYPE_MOUSE 0x00
#define PS2_TYPE_MOUSE_SCROLL 0x01
#define PS2_TYPE_MOUSE_5BTN 0x2
#define PS2_TYPE_KEYBOARD 0x3

drv_type_input_t *contextStruct;

void _mdrvmain()
{
    printf("started experimental ps2 driver!\n");

    contextStruct = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

    // some trickery to simulate some key presses until we implement the actual driver code
    const char *toWrite = "ls\n";

    for (int i = 0; i < strlen(toWrite); i++)
        contextStruct->keys[i] = toWrite[i];

    // flush the context
    sys_drv_flush(SYS_DRIVER_TYPE_INPUT);

    while (1);
}