#include <drv/ps2.h>
#include <cpu/io.h>
#include <cpu/idt.h>
#include <cpu/pic.h>
#include <sched/scheduler.h>
#include <subsys/input.h>
#include <fw/acpi.h>

// translation table for the scan code set 1
char scanCodeSet1[] = "\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
bool controllerPresent = false;

bool port1Present = false;
uint8_t port1Type = PS2_TYPE_INVALID;

bool port2Present = false;
uint8_t port2Type = PS2_TYPE_INVALID;

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
    for (uint32_t timeout = 0; timeout < 0xFFFFF; timeout++) // wait for the output bit to be set in the status register
        if (status() & 1)
            break;
}

// wait for input ready
ifunc void waitInput()
{
    for (uint32_t timeout = 0; timeout < 0xFFFFF; timeout++) // wait for the input bit to not be set in the status register
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
    if (scancode <= sizeof(scanCodeSet1))
        kbAppendChar(scanCodeSet1[scancode - 1]); // call the input subsystem
}

// intrerrupt handlers
extern void PS2Port1HandlerEntry();
extern void PS2Port2HandlerEntry();

void ps2Port1Handler()
{
    if (schedulerEnabled()) // swap the page table
        vmmSwap(vmmGetBaseTable());

    uint8_t data = inb(PS2_DATA);
    if (port1Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);

    if (schedulerEnabled()) // swap the page table back
        vmmSwap(schedulerGetCurrent()->pageTable);

    picEOI();
}

void ps2Port2Handler()
{
    if (schedulerEnabled()) // swap the page table
        vmmSwap(vmmGetBaseTable());

    uint8_t data = inb(PS2_DATA);
    if (port2Type == PS2_TYPE_KEYBOARD) // redirect data to the keyboard handler
        kbHandle(data);

    if (schedulerEnabled()) // swap the page table back
        vmmSwap(schedulerGetCurrent()->pageTable);
    picEOI();
}

// initialize the controller
void ps2Init()
{
    acpi_fadt_t *fadt = (acpi_fadt_t *)acpiGet("FADT");
    if(fadt)
    {
        if(!(fadt->bootFlags & 0b10)) // detect if the pic chips are missing
            return;
    } 

    // disable intrerrupts
    cli();

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
    controllerPresent = output() == 0x55; // if the controller replied with OK it means that it's present and it's working

    if (!controllerPresent) // return since we don't have to do anything if the controller isn't present
        return;

    // test the first port
    command(0xAB);
    waitResponse();
    port1Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    // test the second port
    command(0xA9);
    waitResponse();
    port2Present = output() == 0x0; // if the controller replied with OK it means that the port is present and working

    if (!port1Present && !port2Present) // give up if there aren't any port present
        return;

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

#ifdef K_PS2_DEBUG
        const char *lookup[] = {"mouse", "mouse w/ scroll", "5 button mouse", "keyboard"};
        printks("ps2: detected %s in port 1\n\r", lookup[port1Type]);
#endif
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

#ifdef K_PS2_DEBUG
        const char *lookup[] = {"mouse", "mouse w/ scroll", "5 button mouse", "keyboard"};
        printks("ps2: detected %s in port 2\n\r", lookup[port2Type]);
#endif
    }

    // set the intrerrupt handlers
    if (port1Present)
        idtSetGate((void *)PS2Port1HandlerEntry, PIC_IRQ_1, IDT_InterruptGateU, true);
    if (port2Present)
        idtSetGate((void *)PS2Port2HandlerEntry, PIC_IRQ_12, IDT_InterruptGateU, true);

    // unmask the irqs
    if (port1Present)
        outb(PIC_MASTER_DAT, 0b11111100); // IRQ 1 and IRQ 0 (timer and PS/2 port 1)
    if (port2Present)
        outb(PIC_SLAVE_DAT, 0b11101111); // IRQ 12 (PS/2 port 2)

    // enable intrerrupts
    sti();

    // initialize the keyboard
    kbInit();

#ifdef K_PS2_DEBUG
    printks("ps2: initialized controller and detected %d port(s).\n\r", (uint8_t)(port1Present + port2Present));
#endif
}