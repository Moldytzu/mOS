#include <ps2.h>

uint16_t mouseX;
uint16_t mouseY;

uint8_t packet[3];
uint8_t packetState;

void mouseInit()
{
    memset(packet, 0, 3);
    packetState = 0;

    mouseX = mouseY = 0;

    if (port1Type == PS2_TYPE_MOUSE)
    {
        port1Write(0xF4); // enable data reporting
        flush();
    }

    else if (port2Type == PS2_TYPE_MOUSE)
    {
        port2Write(0xF4); // enable data reporting
        flush();
    }
}

void mouseHandle(uint8_t scancode)
{
    packet[packetState++] = scancode;

    if (packetState == 3)
    {
        mouseX += packet[1];
        mouseY += packet[2];
        packetState = 0;
    }
}