#include <ps2.h>

#define MAX_DELTA_PER_PACKET 50 // fixme: we shouldn't have this

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
    memset(mousePacket, 0, 3);
    packetState = 0;

    if (port1Type == PS2_TYPE_MOUSE)
    {
        port1Write(0xF6); // set defaults
        flush();

        port1Write(0xE6); // set scaling to 1:1
        flush();

        port1Write(0xE8); // set resolution
        flush();
        port1Write(0x00); // 1 count/mm
        flush();

        port1Write(0xF3); // set sample rate
        flush();
        port1Write(200);
        flush();

        port1Write(0xF4); // enable data reporting
        flush();
    }

    else if (port2Type == PS2_TYPE_MOUSE)
    {
        port2Write(0xF6); // set defaults
        flush();

        port2Write(0xE6); // set scaling to 1:1
        flush();

        port2Write(0xE8); // set resolution
        flush();
        port2Write(0x00); // 1 count/mm
        flush();

        port2Write(0xF3); // set sample rate
        flush();
        port2Write(200);
        flush();

        port2Write(0xF4); // enable data reporting
        flush();
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

        // HACK: cap the x and y axis movement so the cursor doesn't fly around
        // we need it for qemu because the emulator wouldn't set the sampling rates and the resolution correctly
        int x = min(packet->xMovement, MAX_DELTA_PER_PACKET);
        int y = min(packet->yMovement, MAX_DELTA_PER_PACKET);

        if (packet->xOverflow || packet->yOverflow)
            return;

        if (packet->xSign)
            x = -x;

        if (!packet->ySign)
            y = -y;

        contextStruct->mouseX += x;
        contextStruct->mouseY += y;
        packetState = 0;
    }
}