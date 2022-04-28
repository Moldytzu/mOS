#include <ps2.h>
#include <io.h>
#include <idt.h>
#include <pic.h>

bool controllerPresent = false;
bool port1Present = false;
bool port2Present = false;

// intrerrupt handlers
extern void PS2Port1HandlerEntry();
extern void PS2Port2HandlerEntry();

void ps2Port1Handler()
{
    uint8_t data = inb(PS2_DATA);
    printk("%c",data);
}

void ps2Port2Handler()
{
    uint8_t data = inb(PS2_DATA);
    printk("%c",data);
}

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

// send configuration byte to the controller
ifunc uint8_t config(uint8_t data)
{
    waitInput();
    command(0x60); // write next byte to controller configuration byte
    waitInput();
    write(data); // write the configuration byte
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

void ps2Init()
{
    // disable intrerrupts
    cli();

    // disable the devices
    command(0xAD);
    waitInput();

    command(0xA7);
    waitInput();

    // flush the output buffer
    output();

    // send configuration byte
    config(0b00110100); // irqs and translation disabled

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

    // set the intrerrupt handlers
    if (port1Present)
        idtSetGate((void *)PS2Port1HandlerEntry, PIC_IRQ_1, IDT_InterruptGateU, true);
    if (port2Present)
        idtSetGate((void *)PS2Port2HandlerEntry, PIC_IRQ_12, IDT_InterruptGateU, true);

    // set new configuration byte
    uint8_t configByte = 0b00000100;
    if(port1Present)
        configByte |= 0b00010001; // set irq and clock for first port
    if(port2Present)
        configByte |= 0b00100010; // set irq and clock for second port
    config(configByte); // send the configuration byte

    // unmask the irq pins
    if(port1Present)
        outb(PIC_MASTER_DAT, 0b11111100); // IRQ 1 and IRQ 0 (timer and PS/2 port 1)
    if(port2Present)
        outb(PIC_SLAVE_DAT, 0b11101111); // IRQ 12 (PS/2 port 2)

    // enable intrerrupts
    sti();

#ifdef K_PS2_DEBUG
    printks("ps2: initialized controller and detected %d ports. using 0x%x as the configuration byte\n\r", (uint8_t)(port1Present + port2Present), configByte);
#endif
}