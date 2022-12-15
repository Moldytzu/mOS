#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <string.h>

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
#define PS2_CTRL_READ_CFG 0x20
#define PS2_CTRL_WRITE_CFG 0x60
#define PS2_CTRL_DISABLE_P2 0xA7
#define PS2_CTRL_ENABLE_P2 0xA8
#define PS2_CTRL_TEST_P2 0xA9
#define PS2_CTRL_TEST_P1 0xAB
#define PS2_CTRL_TEST_CTRL 0xAA
#define PS2_CTRL_ENABLE_P1 0xAE
#define PS2_CTRL_DISABLE_P1 0xAD
#define PS2_CTRL_READ_OUTPUT 0xD0
#define PS2_CTRL_WRITE_P2 0xD4

// translation table for the scan code set 1
char scanCodeSet1[] = "\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
bool controllerPresent = false;

bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

drv_type_input_t *contextStruct;

// macro functions
#define status() inb(PS2_STATUS)
#define output() inb(PS2_DATA)
#define write(data) outb(PS2_DATA, data)
#define command(cmd) outb(PS2_COMMAND, cmd)
#define port1Write(data) write(data)
#define port2Write(data)            \
    {                               \
        command(PS2_CTRL_WRITE_P2); \
        write(data);                \
    }

// initialize the keyboard
void kbInit()
{
    // write to the right port
    if (port1Type == PS2_TYPE_KEYBOARD)
        port1Write(0xF6); // set default parameters
    else if (port2Type == PS2_TYPE_KEYBOARD)
        port2Write(0xF6); // set default parameters

    output(); // flush the buffer
}

// keyboard scancode handler
void kbHandle(uint8_t scancode)
{
    if (scancode <= sizeof(scanCodeSet1))
    {
        // find the first empty in the buffer
        int i = 0;
        for (; i < 16; i++)
            if (!contextStruct->keys[i])
                break;

        contextStruct->keys[i] = scanCodeSet1[scancode - 1]; // set the key at that index
    }
}

void ps2Port1Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port1Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);

    picEOI();
}

void ps2Port2Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port2Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);

    picEOI();
}

const char *lookup[] = {"mouse", "mouse w/ scroll", "5 button mouse", "keyboard"};

// initialize the controller
bool initController()
{
    // todo: interract with the mouse

    // disable the devices
    command(0xAD);
    command(0xA7);

    // flush the output buffer
    output();

    // perform self-test
    command(0xAA);

    if (output() != 0x55) // if the controller didn't reply with OK it means that it isn't present
        return false;

    // test the first port
    command(PS2_CTRL_TEST_P1);
    port1Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    command(PS2_CTRL_TEST_P2);
    port2Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return false;

    // enable the devices
    if (port1Present)
    {
        command(PS2_CTRL_ENABLE_P1);     // enable the first port if it's present
        port1Write(0xFF);                // reset device
        port1Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    if (port2Present)
    {
        command(PS2_CTRL_ENABLE_P2);     // enable the second port if it's present
        port2Write(0xFF);                // reset device
        port2Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    // detect the device types
    if (port1Present)
    {
        port1Write(0xF5); // send disable scanning
        port1Write(0xF2); // send identify

        uint8_t reply[2] = {0, 0};
        reply[0] = output(); // flush the buffer
        reply[1] = output(); // flush the buffer

        port1Write(0xF4); // send enable scanning

        // decode the reply bytes
        if (reply[1] == 0x00)
            port1Type = PS2_TYPE_MOUSE;
        else if (reply[1] == 0x03)
            port1Type = PS2_TYPE_MOUSE_SCROLL;
        else if (reply[1] == 0x04)
            port1Type = PS2_TYPE_MOUSE_5BTN;
        else
            port1Type = PS2_TYPE_KEYBOARD;

        printf("ps2: detected %s in port 1\n\r", lookup[port1Type]);
    }

    if (port2Present)
    {
        port2Write(0xF5); // send disable scanning
        port2Write(0xF2); // send identify

        uint8_t reply[2] = {0, 0};

        reply[0] = output(); // flush the buffer
        reply[1] = output(); // flush the buffer

        port2Write(0xF4); // send enable scanning

        // decode the reply bytes
        if (reply[1] == 0x00)
            port2Type = PS2_TYPE_MOUSE;
        else if (reply[1] == 0x03)
            port2Type = PS2_TYPE_MOUSE_SCROLL;
        else if (reply[1] == 0x04)
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

    // initialize the keyboard
    kbInit();

    printf("ps2: detected %d ports\n", (uint8_t)(port1Present + port2Present));
    return true;
}

void _mdrvmain()
{
    printf("ps2: started experimental ps2 driver!\n");

    contextStruct = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

    if (!initController()) // initialise the controller
        return;

    // flush the context endlessly
    while (1)
    {
        sys_drv_flush(SYS_DRIVER_TYPE_INPUT); // flush the context
        sys_yield();                          // yield
    }
}