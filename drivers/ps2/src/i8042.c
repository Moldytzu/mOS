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
    i8042SendCommand(PS2_CTRL_READ_CFG);
    i8042WaitOutputBuffer();
    return i8042ReadOutput();
}

void ps2WriteConfigByte(uint8_t byte)
{
    i8042SendCommand(PS2_CTRL_WRITE_CFG);
    i8042FlushBuffers();
    i8042WriteData(byte);
    i8042FlushBuffers();
}

// perform controller self-test
bool ps2SelfTest()
{
    i8042SendCommand(PS2_CTRL_SELF_TEST); // send test command
    i8042WaitOutputBuffer();                // wait for the test to be done
    return i8042ReadOutput() == 0x55;     // 0x55 is success
}

// initialize the controller
bool ps2InitController()
{
    // disable the devices
    i8042SendCommand(PS2_CTRL_DISABLE_P1);
    i8042FlushBuffers();

    i8042SendCommand(PS2_CTRL_DISABLE_P2);
    i8042FlushBuffers();

    // alter config byte
    uint8_t configByte = ps2ReadConfigByte();
    configByte &= ~(0b11);   // disable IRQs
    configByte |= 0b1000000; // enable scan code translation
    ps2WriteConfigByte(configByte);

    // perform self-test
    if (!ps2SelfTest())
        ps2Trace("controller failed self-test or isn't present"); // HACK: on some computers the test fails for some reason (including mine)

    // test the first port
    i8042SendCommand(PS2_CTRL_TEST_P1);
    i8042WaitOutputBuffer();
    port1Present = i8042ReadOutput() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    i8042SendCommand(PS2_CTRL_TEST_P2);
    i8042WaitOutputBuffer();
    port2Present = i8042ReadOutput() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return ps2Trace("failed to detect ports");

    // enable the devices
    if (port1Present)
    {
        i8042SendCommand(PS2_CTRL_ENABLE_P1);                     // enable the first port if it's present
        i8042FlushBuffers();                                         // flush output
        port1Write(PS2_DEV_RESET);                       // reset device
        i8042WaitOutputBuffer();                                    // wait for the status
        port1Present = i8042ReadOutput() == 0xFA;                 // if the controller replied with OK it means that a device is in that port
        i8042WaitOutputBuffer();                                    // wait for self test status
        port1Present = port1Present && i8042ReadOutput() == 0xAA; // device sends 0xAA on success
    }

    if (port2Present)
    {
        i8042SendCommand(PS2_CTRL_ENABLE_P2);                     // enable the second port if it's present
        i8042FlushBuffers();                                         // flush output
        port2Write(PS2_DEV_RESET);                       // reset device
        i8042WaitOutputBuffer();                                    // wait for the status
        port2Present = i8042ReadOutput() == 0xFA;                 // if the controller replied with OK it means that a device is in that port
        i8042WaitOutputBuffer();                                    // wait for self test status
        port2Present = port2Present && i8042ReadOutput() == 0xAA; // device sends 0xAA on success
    }

    // detect the device types
    if (port1Present)
    {
        port1Write(PS2_DEV_DISABLE_SCANNING); // send disable scanning
        i8042FlushBuffers();

        port1Write(PS2_DEV_IDENTIFY); // send identify
        i8042FlushBuffers();

        // gather response
        uint8_t reply[2] = {0, 0};
        i8042WaitOutputBuffer();
        reply[0] = i8042ReadOutput();
        i8042WaitOutputBuffer();
        reply[1] = i8042ReadOutput();

        // decode the reply bytes
        port1Type = ps2DecodeBytes(reply);

        port1Write(PS2_DEV_ENABLE_SCANNING); // send enable scanning
        i8042FlushBuffers();

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
        i8042FlushBuffers();

        port2Write(PS2_DEV_IDENTIFY); // send identify
        i8042FlushBuffers();

        // gather response
        uint8_t reply[2] = {0, 0};
        i8042WaitOutputBuffer();
        reply[0] = i8042ReadOutput();
        i8042WaitOutputBuffer();
        reply[1] = i8042ReadOutput();

        // decode the reply bytes
        port2Type = ps2DecodeBytes(reply);

        port2Write(PS2_DEV_ENABLE_SCANNING); // send enable scanning
        i8042FlushBuffers();

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