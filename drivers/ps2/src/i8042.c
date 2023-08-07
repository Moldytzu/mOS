#include <ps2.h>

bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

drv_type_input_t *contextStruct;

void ps2Port1Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port1Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);
    else if (port1Type == PS2_TYPE_MOUSE)
        mouseHandle(data);
}

void ps2Port2Handler()
{
    uint8_t data = inb(PS2_DATA);
    if (port2Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);
    else if (port2Type == PS2_TYPE_MOUSE)
        mouseHandle(data);
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
    else if (reply[0] == 0xAB)
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

    // disable the devices
    command(PS2_CTRL_DISABLE_P1);
    flush();

    command(PS2_CTRL_DISABLE_P2);
    flush();

    // disable irqs in config byte
    command(PS2_CTRL_READ_CFG);
    waitOutput();
    uint8_t cfg = output();
    cfg &= ~(0b11);   // disable IRQ
    cfg |= 0b1000000; // enable translation
    command(PS2_CTRL_WRITE_CFG);
    write(cfg);
    flush();

    // perform self-test
    command(PS2_CTRL_SELF_TEST);
    waitOutput();         // wait for the test to be done
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
        command(PS2_CTRL_ENABLE_P1);     // enable the first port if it's present
        flush();                         // flush output
        port1Write(0xFF);                // reset device
        waitOutput();                    // wait for the status
        port1Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
        flush();                         // the device sends an 0xAA after this
    }

    if (port2Present)
    {
        command(PS2_CTRL_ENABLE_P2);     // enable the second port if it's present
        flush();                         // flush output
        port2Write(0xFF);                // reset device
        waitOutput();                    // wait for the status
        port2Present = output() == 0xFA; // if the controller replied with OK it means that a device is in that port
        flush();                         // the device sends an 0xAA after this
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

    // initialize the devices
    kbInit();
    mouseInit();

    printf("ps2: detected %d ports\n", (uint8_t)(port1Present + port2Present));
    return true;
}

void setupHandlers()
{
    // set the intrerrupt handlers
    if (port1Present)
    {
        uint8_t vector = sys_driver_allocate_vector();
        sys_idt_set(ps2Port1Handler, vector);
        sys_driver(SYS_DRIVER_REDIRECT_IRQ_TO_VECTOR, 1, vector, 0);
    }

    if (port2Present)
    {
        uint8_t vector = sys_driver_allocate_vector();
        sys_idt_set(ps2Port2Handler, vector);
        sys_driver(SYS_DRIVER_REDIRECT_IRQ_TO_VECTOR, 12, vector, 0);
    }
}

void _mdrvmain()
{
    if (!initController()) // initialise the controller
    {
        puts("ps2: failed to initialise!\n");
        abort();
    }

    contextStruct = (drv_type_input_t *)sys_driver_announce(SYS_DRIVER_TYPE_INPUT); // announce that we are an input-related driver

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
        // printf("%d %d\n", mouseX, mouseY);
        sys_driver_flush(SYS_DRIVER_TYPE_INPUT); // flush the context
        sys_yield();                             // yield
    }
}