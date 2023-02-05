#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <string.h>

// ports
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

// types
#define PS2_TYPE_INVALID 0x4
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

// constants
#define PS2_TIMEOUT_YIELDS 10

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
#define waitOutput() for (int i = 0; i < 100 && status() & 0b1; i++)
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

const char *lookup[] = {"mouse", "mouse w/ scroll", "5 button mouse", "keyboard", "unknown"};
int ps2DecodeBytes(uint8_t *reply)
{
    if (reply[0] == 0x00)
        return PS2_TYPE_MOUSE;
    else if (reply[0] == 0x03)
        return PS2_TYPE_MOUSE_SCROLL;
    else if (reply[0] == 0x04)
        return PS2_TYPE_MOUSE_5BTN;
    else if (reply[0] == 0xAB || reply[0] == 0xAA)
        return PS2_TYPE_KEYBOARD;
    else
        return PS2_TYPE_INVALID;
}

// initialize the controller
bool initController()
{
    // todo: interract with the mouse

    // disable the devices
    command(PS2_CTRL_DISABLE_P1);
    command(PS2_CTRL_DISABLE_P2);

    // flush the output buffer
    output();

    // disable irqs in config byte
    {
        command(PS2_CTRL_READ_CFG);
        waitOutput();
        uint8_t cfg = output();
        cfg &= ~(0b11);   // disable IRQ
        cfg |= 0b1000000; // enable translation
        command(PS2_CTRL_WRITE_CFG);
        write(cfg);
        output();
    }

    // perform self-test
    command(0xAA);

    waitOutput(); // wait for the test to be done

    if (output() != 0x55) // if the controller didn't reply with OK it means that it isn't present
        return false;

    // test the first port
    command(PS2_CTRL_TEST_P1);
    waitOutput();
    port1Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    command(PS2_CTRL_TEST_P2);
    waitOutput();
    port2Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return false;

    // enable the devices
    if (port1Present)
    {
        command(PS2_CTRL_ENABLE_P1); // enable the first port if it's present
        port1Write(0xFF);            // reset device
        waitOutput();
        port1Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    if (port2Present)
    {
        command(PS2_CTRL_ENABLE_P2); // enable the second port if it's present
        port2Write(0xFF);            // reset device
        waitOutput();
        port2Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
    }

    // detect the device types
    if (port1Present)
    {
        port1Write(0xF5); // send disable scanning
        waitOutput();
        output();

        port1Write(0xF2); // send identify
        waitOutput();
        output();

        uint8_t reply[2] = {0, 0};
        reply[0] = output(); // fill the buffer with the response word
        waitOutput();
        reply[1] = output();

        port1Write(0xF4); // send enable scanning
        waitOutput();
        output();

        // decode the reply bytes
        port1Type = ps2DecodeBytes(reply);

        printf("ps2: detected type %s (%x %x) in port 1\n", lookup[port1Type], reply[0], reply[1]);
    }

    if (port2Present)
    {
        port2Write(0xF5); // send disable scanning
        waitOutput();
        output();

        port2Write(0xF2); // send identify
        waitOutput();
        output();

        uint8_t reply[2] = {0, 0};
        reply[0] = output(); // fill the buffer with the response word
        waitOutput();
        reply[1] = output();

        port2Write(0xF4); // send enable scanning
        waitOutput();
        output();

        // decode the reply bytes
        port2Type = ps2DecodeBytes(reply);

        printf("ps2: detected type %s (%x %x) in port 2\n", lookup[port2Type], reply[0], reply[1]);
    }

    // enable irqs in config byte for the detected devices
    {
        command(PS2_CTRL_READ_CFG);
        uint8_t cfg = output();

        // enable irqs for the detected ports
        if (port1Present)
            cfg |= 0b1;

        if (port2Present)
            cfg |= 0b10;

        command(PS2_CTRL_WRITE_CFG);
        write(cfg);
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
    contextStruct = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

    if (!initController()) // initialise the controller
        return;

    printf("ps2: started ps2 driver!\n");

    // flush the context endlessly
    while (1)
    {
        sys_drv_flush(SYS_DRIVER_TYPE_INPUT); // flush the context
        sys_yield();                          // yield
    }
}