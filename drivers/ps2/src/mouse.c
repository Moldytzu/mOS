#include <ps2.h>

// commands
#define PS2_MS_SET_SCALING 0xE6
#define PS2_MS_SET_RESOLUTION 0xE8
#define PS2_MS_SET_SAMPLE_RATE 0xF3
#define PS2_MS_ENABLE_DATA_REPORTING 0xF4

pstruct
{
    unsigned buttonLeft : 1;
    unsigned buttonRight : 1;
    unsigned buttonMiddle : 1;
    unsigned reserved : 1;
    unsigned xSign : 1;
    unsigned ySign : 1;
    unsigned yOverflow : 1;
    unsigned xOverflow : 1;

    uint8_t xMovement;
    uint8_t yMovement;
}
ps2_mouse_packet_t;

uint8_t mousePacket[3];
uint8_t packetState;

void mouseInit()
{
    packetState = 0;

    if (port1Type == PS2_TYPE_MOUSE)
    {
        port1Write(PS2_DEV_SET_DEFAULTS); // set defaults
        i8042FlushBuffers();

        port1Write(PS2_MS_SET_SCALING); // set scaling to 1:1
        i8042FlushBuffers();

        port1Write(PS2_MS_SET_RESOLUTION); // set resolution
        i8042FlushBuffers();
        port1Write(0x00); // 1 count/mm
        i8042FlushBuffers();

        port1Write(PS2_MS_SET_SAMPLE_RATE); // set sample rate
        i8042FlushBuffers();
        port1Write(200);
        i8042FlushBuffers();

        port1Write(PS2_MS_ENABLE_DATA_REPORTING); // enable data reporting
        i8042FlushBuffers();
    }

    else if (port2Type == PS2_TYPE_MOUSE)
    {
        port2Write(PS2_DEV_SET_DEFAULTS); // set defaults
        i8042FlushBuffers();

        port2Write(PS2_MS_SET_SCALING); // set scaling to 1:1
        i8042FlushBuffers();

        port2Write(PS2_MS_SET_RESOLUTION); // set resolution
        i8042FlushBuffers();
        port2Write(0x00); // 1 count/mm
        i8042FlushBuffers();

        port2Write(PS2_MS_SET_SAMPLE_RATE); // set sample rate
        i8042FlushBuffers();
        port2Write(200);
        i8042FlushBuffers();

        port2Write(PS2_MS_ENABLE_DATA_REPORTING); // enable data reporting
        i8042FlushBuffers();
    }
}

void mouseHandle(uint8_t scancode)
{
    mousePacket[packetState++] = scancode;

    // the mouse sends a packet that is 3 bytes long
    // thus we need 3 states
    if (packetState == 3)
    {
        ps2_mouse_packet_t *packet = (ps2_mouse_packet_t *)mousePacket;

        int x = packet->xMovement;
        int y = packet->yMovement;

        // I don't know how to handle this. fixme: find out how
        if (packet->xOverflow || packet->yOverflow)
        {
            packetState = 0;
            return;
        }

        // handle sign
        if (packet->xSign)
            x -= 0x100;

        if (packet->ySign)
            y -= 0x100;

        // fill context struct metadata
        contextStruct->mouseX += x;
        contextStruct->mouseY += -y;
        contextStruct->mouseButtons[MOUSE_BUTTON_LEFT] = packet->buttonLeft;
        contextStruct->mouseButtons[MOUSE_BUTTON_MIDDLE] = packet->buttonMiddle;
        contextStruct->mouseButtons[MOUSE_BUTTON_RIGHT] = packet->buttonRight;
        packetState = 0;
    }
}