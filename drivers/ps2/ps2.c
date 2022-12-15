#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <string.h>

/*
EXPERIMENTAL CODE! DONT TOUCH IF YOU DONT KNOW WHAT ARE YOU DOING (says moldu to moldu)
*/

// ports
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

// types
#define PS2_TYPE_INVALID 0xFF
#define PS2_TYPE_MOUSE 0x00
#define PS2_TYPE_MOUSE_SCROLL 0x01
#define PS2_TYPE_MOUSE_5BTN 0x2
#define PS2_TYPE_KEYBOARD 0x3

// commands

// translation table for the scan code set 1
char scanCodeSet1[] = "\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
bool controllerPresent = false;

bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

drv_type_input_t *contextStruct;
uint64_t pageTable = 0;

// get status register of the controller
ifunc uint8_t status()
{
    return inb(PS2_STATUS);
}

// get output buffer of the controller
ifunc uint8_t output()
{
    return inb(PS2_DATA);
}

// wait for response
ifunc void waitResponse()
{
    for (uint32_t timeout = 0; timeout < 0xFFFF; timeout++) // wait for the output bit to be set in the status register
        if (status() & 1)
            break;
}

// wait for input ready
ifunc void waitInput()
{
    for (uint32_t timeout = 0; timeout < 0xFFFF; timeout++) // wait for the input bit to not be set in the status register
        if (!(status() & 2))
            break;
}

// send data to the controller
ifunc void write(uint8_t data)
{
    outb(PS2_DATA, data);
}

// send command to the controller
ifunc void command(uint8_t cmd)
{
    outb(PS2_COMMAND, cmd);
}

// send data to the first port
ifunc void port1Write(uint8_t data)
{
    waitInput();
    // send data to the port
    write(data);
}

// send data to the second port
ifunc void port2Write(uint8_t data)
{
    // tell the controller that we're sending data to the second port
    command(0xD4);
    waitInput();
    // send data to the port
    write(data);
}

// initialize the keyboard
void kbInit()
{
    // write to the right port
    if (port1Type == PS2_TYPE_KEYBOARD)
        port1Write(0xF6); // set default parameters
    else if (port2Type == PS2_TYPE_KEYBOARD)
        port2Write(0xF6); // set default parameters

    waitResponse(); // wait for the reply
    output();       // flush the buffer
}

// keyboard scancode handler
void kbHandle(uint8_t scancode)
{
    serialWritec(scancode);
    // if (scancode <= sizeof(scanCodeSet1))
    //    contextStruct->keys[0] = scanCodeSet1[scancode - 1]; // set the key; todo: also fill the rest of the buffer
}

void ps2Port1Handler()
{
    uint8_t data = inb(PS2_DATA);
    while (1)
        ;
    // if (port1Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
    //     kbHandle(data);

    picEOI();
}

void ps2Port2Handler()
{
    uint8_t data = inb(PS2_DATA);
    // if (port2Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
    //     kbHandle(data);

    picEOI();
}

const char *lookup[] = {"mouse", "mouse w/ scroll", "5 button mouse", "keyboard"};
// initialize the controller
bool initController()
{
    // todo: interract with the mouse

    // disable intrerrupts
    // cli();

    // disable the devices
    command(0xAD);
    waitInput();

    command(0xA7);
    waitInput();

    // flush the output buffer
    output();

    // perform self-test
    command(0xAA);
    waitResponse();

    if (output() != 0x55) // if the controller didn't reply with OK it means that it isn't present
        return false;

    // test the first port
    command(0xAB);
    waitResponse();
    port1Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    command(0xA9);
    waitResponse();
    port2Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return false;

    // enable the devices
    if (port1Present)
    {
        command(0xAE); // enable the first port if it's present
        waitResponse();

        port1Write(0xFF); // reset device
        waitResponse();
        port1Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    if (port2Present)
    {
        command(0xA8); // enable the second port if it's present
        waitResponse();

        port2Write(0xFF); // reset device
        waitResponse();
        port2Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    // detect the device types
    if (port1Present)
    {
        port1Write(0xF5); // send disable scanning
        waitResponse();   // wait for acknoledgement
        output();         // flush the buffer
        port1Write(0xF2); // send identify
        waitResponse();   // wait for acknoledgement
        output();         // flush the buffer

        uint8_t reply[2] = {0, 0};
        waitResponse();      // wait for the reply
        reply[0] = output(); // flush the buffer
        waitResponse();      // wait for the reply
        reply[1] = output(); // flush the buffer

        port1Write(0xF4); // send enable scanning
        waitResponse();   // wait for acknoledgement

        // decode the reply bytes
        if (reply[0] == 0x00 && reply[1] == 0x00)
            port1Type = PS2_TYPE_MOUSE;
        else if (reply[0] == 0x03 && reply[1] == 0x00)
            port1Type = PS2_TYPE_MOUSE_SCROLL;
        else if (reply[0] == 0x04 && reply[1] == 0x00)
            port1Type = PS2_TYPE_MOUSE_5BTN;
        else
            port1Type = PS2_TYPE_KEYBOARD;

        printf("ps2: detected %s in port 1\n\r", lookup[port1Type]);
    }

    if (port2Present)
    {
        port2Write(0xF5); // send disable scanning
        waitResponse();   // wait for acknoledgement
        output();         // flush the buffer
        port2Write(0xF2); // send identify
        waitResponse();   // wait for acknoledgement
        output();         // flush the buffer

        uint8_t reply[2] = {0, 0};
        waitResponse();      // wait for the reply
        reply[0] = output(); // flush the buffer
        waitResponse();      // wait for the reply
        reply[1] = output(); // flush the buffer

        port2Write(0xF4); // send enable scanning
        waitResponse();   // wait for acknoledgement

        // decode the reply bytes
        if (reply[0] == 0x00 && reply[1] == 0x00)
            port2Type = PS2_TYPE_MOUSE;
        else if (reply[0] == 0x03 && reply[1] == 0x00)
            port2Type = PS2_TYPE_MOUSE_SCROLL;
        else if (reply[0] == 0x04 && reply[1] == 0x00)
            port2Type = PS2_TYPE_MOUSE_5BTN;
        else
            port2Type = PS2_TYPE_KEYBOARD;

        printf("ps2: detected %s in port 2\n\r", lookup[port2Type]);
    }

    // set the intrerrupt handlers
    if (port1Present)
        sys_idt_set(ps2Port1Handler, PIC_IRQ_1);
    if (port2Present)
        sys_idt_set(ps2Port2Handler, PIC_IRQ_12);

    // unmask the irqs
    if (port1Present)
        outb(PIC_MASTER_DAT, 0b11111100); // IRQ 1 and IRQ 0 (timer and PS/2 port 1)
    if (port2Present)
        outb(PIC_SLAVE_DAT, 0b11101111); // IRQ 12 (PS/2 port 2)

    // enable intrerrupts
    // sti();

    // initialize the keyboard
    kbInit();

    printf("ps2: detected %d ports\n", (uint8_t)(port1Present + port2Present));
    return true;
}

void _mdrvmain()
{
    printf("started experimental ps2 driver!\n");

    contextStruct = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

    if (!initController())
        return;

    // some trickery to simulate some key presses until we implement the actual driver code
    const char *toWrite = "ls\n";

    for (int i = 0; i < strlen(toWrite); i++)
        contextStruct->keys[i] = toWrite[i];

    // flush the context
    sys_drv_flush(SYS_DRIVER_TYPE_INPUT);

    while (1)
        ;
}