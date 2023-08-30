#include <ps2.h>

// global variables
bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

drv_type_input_t *contextStruct;

// interrupt handlers
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

// helpers
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

bool ps2Trace(const char *msg) // todo: make this a propper logging function
{
    printf("ps2: %s\n", msg);
    return false;
}

uint8_t ps2ReadConfigByte()
{
    command(PS2_CTRL_READ_CFG);
    waitOutput();
    return output();
}

void ps2WriteConfigByte(uint8_t byte)
{
    command(PS2_CTRL_WRITE_CFG);
    flush();
    write(byte);
    flush();
}

// initialize the controller
bool ps2InitController()
{
    // disable the devices
    command(PS2_CTRL_DISABLE_P1);
    flush();

    command(PS2_CTRL_DISABLE_P2);
    flush();

    // alter config byte
    uint8_t configByte = ps2ReadConfigByte();
    configByte &= ~(0b11);   // disable IRQs
    configByte |= 0b1000000; // enable scan code translation
    ps2WriteConfigByte(configByte);

    // perform self-test
    command(PS2_CTRL_SELF_TEST); // send test command
    waitOutput();                // wait for the test to be done
    if (output() != 0x55)        // if the controller didn't reply with OK it means that it isn't present
        return ps2Trace("controller failed self-test or isn't present");

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
        command(PS2_CTRL_ENABLE_P1);                     // enable the first port if it's present
        flush();                                         // flush output
        port1Write(PS2_DEV_RESET);                       // reset device
        waitOutput();                                    // wait for the status
        port1Present = output() == 0xFA;                 // if the controller replied with OK it means that a device is in that port
        waitOutput();                                    // wait for self test status
        port1Present = port1Present && output() == 0xAA; // device sends 0xAA on success
    }

    if (port2Present)
    {
        command(PS2_CTRL_ENABLE_P2);                     // enable the second port if it's present
        flush();                                         // flush output
        port2Write(PS2_DEV_RESET);                       // reset device
        waitOutput();                                    // wait for the status
        port2Present = output() == 0xFA;                 // if the controller replied with OK it means that a device is in that port
        waitOutput();                                    // wait for self test status
        port2Present = port2Present && output() == 0xAA; // device sends 0xAA on success
    }

    // detect the device types
    if (port1Present)
    {
        port1Write(PS2_DEV_DISABLE_SCANNING); // send disable scanning
        flush();

        port1Write(PS2_DEV_IDENTIFY); // send identify
        flush();

        // gather response
        uint8_t reply[2] = {0, 0};
        waitOutput();
        reply[0] = output();
        waitOutput();
        reply[1] = output();

        // decode the reply bytes
        port1Type = ps2DecodeBytes(reply);

        port1Write(PS2_DEV_ENABLE_SCANNING); // send enable scanning
        flush();

        printf("ps2: detected type %s (%x %x) in port 1\n", lookup[port1Type], reply[0], reply[1]);

        if (port1Type == PS2_TYPE_INVALID) // this branch is probably not taken
        {
            puts("ps2: guessing keyboard\n");
            port1Type = PS2_TYPE_KEYBOARD;
        }
    }

    if (port2Present)
    {
        port2Write(PS2_DEV_DISABLE_SCANNING); // send disable scanning
        flush();

        port2Write(PS2_DEV_IDENTIFY); // send identify
        flush();

        // gather response
        uint8_t reply[2] = {0, 0};
        waitOutput();
        reply[0] = output();
        waitOutput();
        reply[1] = output();

        // decode the reply bytes
        port2Type = ps2DecodeBytes(reply);

        port2Write(PS2_DEV_ENABLE_SCANNING); // send enable scanning
        flush();

        printf("ps2: detected type %s (%x %x) in port 2\n", lookup[port2Type], reply[0], reply[1]);

        if (port2Type == PS2_TYPE_INVALID) // this branch is probably not taken
        {
            puts("ps2: guessing keyboard\n");
            port2Type = PS2_TYPE_KEYBOARD;
        }
    }

    // initialize the devices
    kbInit();
    mouseInit();

    printf("ps2: detected %d ports\n", (uint8_t)(port1Present + port2Present));
    return true;
}

void ps2SetupIDTHandlers()
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

    // enable irqs in config byte for the detected devices
    uint8_t configByte = ps2ReadConfigByte();

    if (port1Present)
        configByte |= 0b1;

    if (port2Present)
        configByte |= 0b10;

    ps2WriteConfigByte(configByte);
}

void _mdrvmain()
{
    if (!ps2InitController()) // initialise the controller
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

    ps2SetupIDTHandlers(); // set up interrupt handlers

    puts("ps2: started ps2 driver!\n");

    // flush the context endlessly
    while (1)
    {
        sys_driver_flush(SYS_DRIVER_TYPE_INPUT); // flush the context
        sys_yield();                             // yield
    }
}