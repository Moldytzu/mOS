#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#define PS2_TIMEOUT_YIELDS 3

// translation table for the scan code set 1
char scanCodeSet1[] = "\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
char scanCodeSet1Shifted[] = "\e!@#$%^&*()_+\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 ";
bool controllerPresent = false;

bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

drv_type_input_t *contextStruct;

// macro functions
#define waitOutput()                                               \
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b1; i++) \
    {                                                              \
        sys_yield();                                               \
    }
#define waitInput()                                                 \
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b10; i++) \
    {                                                               \
        sys_yield();                                                \
    }
#define status() inb(PS2_STATUS)
#define output() inb(PS2_DATA)
#define write(data)           \
    {                         \
        waitInput();          \
        outb(PS2_DATA, data); \
    }
#define command(cmd)            \
    {                           \
        waitInput();            \
        outb(PS2_COMMAND, cmd); \
    }
#define port1Write(data) \
    {                    \
        waitInput();     \
        write(data);     \
    }
#define port2Write(data)            \
    {                               \
        waitInput();                \
        command(PS2_CTRL_WRITE_P2); \
        waitInput();                \
        write(data);                \
    }
#define flush()       \
    {                 \
        waitOutput(); \
        output();     \
    }

// initialize the keyboard
void kbInit()
{
    // write to the right port
    if (port1Type == PS2_TYPE_KEYBOARD)
    {
        port1Write(0xF6); // set default parameters

        flush(); // flush the buffer

        port1Write(0xF0);           // set scan code set 1
        for (int i = 0; i < 2; i++) // wait for the keyboard to send 0xFA 0xFA
            flush();

        port1Write(0xF3); // set typematic rate
        flush();

        port1Write(0b00100000); // 30hz repeat rate and 500 ms delay for repeat
        flush();
    }
    else if (port2Type == PS2_TYPE_KEYBOARD)
    {
        port2Write(0xF6); // set default parameters

        flush(); // flush the buffer

        port2Write(0xF0);           // set scan code set 1
        for (int i = 0; i < 2; i++) // wait for the keyboard to send 0xFA 0xFA
            flush();

        port2Write(0xF3); // set typematic rate
        flush();

        port2Write(0b00100000); // 30hz repeat rate and 500 ms delay for repeat
        flush();
    }
}

bool isShifted = false;

// keyboard scancode handler
void kbHandle(uint8_t scancode)
{
    if (scancode > 0xDF) // over what we expect
        return;

    // find the first empty in the buffer
    int i = 0;
    for (; i < 16; i++)
        if (!contextStruct->keys[i])
            break;

    if (i == 15) // filled buffer
        return;  // drop key

    if (scancode == 0x3A + 0x80) // caps lock released
    {
        isShifted = !isShifted;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) // left+right shift
    {
        isShifted = true;
        return;
    }

    if (scancode == 0x2A + 0x80 || scancode == 0x36 + 0x80) // left+right shift released
    {
        isShifted = false;
        return;
    }

    if (scancode > sizeof(scanCodeSet1)) // not in scan code set
        return;

    if (isShifted)
        contextStruct->keys[i] = scanCodeSet1Shifted[scancode - 1]; // set the key at that index
    else
        contextStruct->keys[i] = scanCodeSet1[scancode - 1]; // set the key at that index
}

void ps2Port1Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port1Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);
}

void ps2Port2Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port2Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);
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

bool ps2Trace(const char *msg)
{
    printf("ps2: %s\n");
    return false;
}

// initialize the controller
bool initController()
{
    // todo: interract with the mouse
    // todo: fix timings

    // disable the devices
    command(PS2_CTRL_DISABLE_P1);
    flush();

    command(PS2_CTRL_DISABLE_P2);
    flush();

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
        return ps2Trace("controller failed self-test");

    // test the first port
    command(PS2_CTRL_TEST_P1);
    waitOutput();
    port1Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    command(PS2_CTRL_TEST_P2);
    waitOutput();
    port2Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return ps2Trace("failed to detect ports");

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
        flush();

        port1Write(0xF2); // send identify
        flush();

        uint8_t reply[2] = {0, 0};
        waitOutput();
        reply[0] = output(); // fill the buffer with the response word
        waitOutput();
        reply[1] = output();

        port1Write(0xF4); // send enable scanning
        flush();

        // decode the reply bytes
        port1Type = ps2DecodeBytes(reply);

        printf("ps2: detected type %s (%x %x) in port 1\n", lookup[port1Type], reply[0], reply[1]);

        if (port1Type == PS2_TYPE_INVALID)
        {
            puts("ps2: guessing keyboard\n");
            port1Type = PS2_TYPE_KEYBOARD;
        }
    }

    if (port2Present)
    {
        port2Write(0xF5); // send disable scanning
        flush();

        port2Write(0xF2); // send identify
        flush();

        uint8_t reply[2] = {0, 0};
        waitOutput();
        reply[0] = output(); // fill the buffer with the response word
        waitOutput();
        reply[1] = output();

        port2Write(0xF4); // send enable scanning
        flush();

        // decode the reply bytes
        port2Type = ps2DecodeBytes(reply);

        printf("ps2: detected type %s (%x %x) in port 2\n", lookup[port2Type], reply[0], reply[1]);

        if (port2Type == PS2_TYPE_INVALID)
        {
            puts("ps2: guessing keyboard\n");
            port2Type = PS2_TYPE_KEYBOARD;
        }
    }

    // enable irqs in config byte for the detected devices
    {
        command(PS2_CTRL_READ_CFG);
        waitOutput();
        uint8_t cfg = output();

        // enable irqs for the detected ports
        if (port1Present)
            cfg |= 0b1;

        if (port2Present)
            cfg |= 0b10;

        command(PS2_CTRL_WRITE_CFG);
        waitOutput();
        write(cfg);
    }

    // initialize the keyboard
    kbInit();

    printf("ps2: detected %d ports\n", (uint8_t)(port1Present + port2Present));
    return true;
}

void setupHandlers()
{
    // set the intrerrupt handlers
    if (port1Present)
    {
        sys_idt_set(ps2Port1Handler, 0x21); // todo: allocate idt vectors
        sys_driver(SYS_DRIVER_REDIRECT_IRQ_TO_VECTOR, 1, 0x21, 0);
    }

    if (port2Present)
    {
        sys_idt_set(ps2Port2Handler, 0x22); // todo: allocate idt vectors
        sys_driver(SYS_DRIVER_REDIRECT_IRQ_TO_VECTOR, 12, 0x22, 0);
    }
}

void _mdrvmain()
{
    isShifted = false;

    if (!initController()) // initialise the controller
    {
        puts("ps2: failed to initialise!\n");
        abort();
    }

    contextStruct = (drv_type_input_t *)sys_drv_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

    if (!contextStruct)
    {
        puts("ps2: failed to announce!\n");
        abort();
    }

    setupHandlers();

    puts("ps2: started ps2 driver!\n");

    // flush the context endlessly
    while (1)
    {
        sys_drv_flush(SYS_DRIVER_TYPE_INPUT); // flush the context
        sys_yield();                          // yield
    }
}