#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ids
#define SVGA_PCI_VENDOR 0x15AD
#define SVGA_PCI_DEVICE 0x0405

// port offsets
#define SVGA_INDEX 0
#define SVGA_VALUE 1
#define SVGA_BIOS 2
#define SVGA_IRQSTATUS 0x8

// registers
#define SVGA_REG_ID 0
#define SVGA_REG_ENABLE 1
#define SVGA_REG_WIDTH 2
#define SVGA_REG_HEIGHT 3
#define SVGA_REG_MAX_WIDTH 4
#define SVGA_REG_MAX_HEIGHT 5
#define SVGA_REG_BPP 7
#define SVGA_REG_FB_OFFSET 14
#define SVGA_REG_VRAM_SIZE 15
#define SVGA_REG_FB_SIZE 16
#define SVGA_REG_FIFO_SIZE 19
#define SVGA_REG_CONFIG_DONE 20
#define SVGA_REG_SYNC 21
#define SVGA_REG_BUSY 22

// fifo offsets
#define SVGA_FIFO_MIN 0
#define SVGA_FIFO_MAX 1
#define SVGA_FIFO_NEXT 2
#define SVGA_FIFO_STOP 3
#define SVGA_FIFO_RESERVED 14
#define SVGA_FIFO_NUM_REGS 16

// fifo commands
#define SVGA_CMD_UPDATE 1

// magic
#define SVGA_ID_MAGIC 0x900000
#define SVGA_GEN_ID(ver) (SVGA_ID_MAGIC << 8 | (ver))

drv_type_framebuffer_t *fb;
drv_pci_header0_t *device;

uint8_t version = 2;
uint16_t ioBase = 0;

uint32_t maxWidth = 0;
uint32_t maxHeight = 0;

uint32_t fbAddr = 0;
uint32_t fifoAddr = 0;
uint32_t *fifo = 0;

uint32_t fbSize = 0;
uint32_t fifoSize = 0;
uint32_t vramSize = 0;

uint32_t fbOffset = 0;

uint32_t readRegister(uint32_t reg)
{
    outl(ioBase + SVGA_INDEX, reg);  // point to the register
    return inl(ioBase + SVGA_VALUE); // return the data
}

void writeRegister(uint32_t reg, uint32_t value)
{
    outl(ioBase + SVGA_INDEX, reg);   // point to the register
    outl(ioBase + SVGA_VALUE, value); // write the data
}

void waitFIFO()
{
    writeRegister(SVGA_REG_SYNC, 1);    // enter syncronisation
    while (readRegister(SVGA_REG_BUSY)) // wait for the busy flag to be clear
        sys_yield();
}

void resetFIFO()
{
    // reset fifo to some default values
    fifo = (uint32_t *)(uint64_t)fifoAddr;
    fifo[SVGA_FIFO_MIN] = 293 * sizeof(uint32_t); // number of registers
    fifo[SVGA_FIFO_MAX] = fifoSize;               // size
    fifo[SVGA_FIFO_NEXT] = fifo[SVGA_FIFO_MIN];   // next command
    fifo[SVGA_FIFO_STOP] = fifo[SVGA_FIFO_MIN];   // stop at command

    writeRegister(SVGA_REG_CONFIG_DONE, 1);
}

void writeFIFO(uint16_t data)
{
    if (((fifo[SVGA_FIFO_NEXT] == fifo[SVGA_FIFO_MAX] - 4) && fifo[SVGA_FIFO_STOP] == fifo[SVGA_FIFO_MIN]) ||
        (fifo[SVGA_FIFO_NEXT] + 4 == fifo[SVGA_FIFO_STOP]))
        waitFIFO();

    fifo[fifo[SVGA_FIFO_NEXT]] = data;
    fifo[SVGA_FIFO_NEXT] += 4;

    if (fifo[SVGA_FIFO_NEXT] == fifo[SVGA_FIFO_MAX])
        fifo[SVGA_FIFO_NEXT] = fifo[SVGA_FIFO_MIN];
}

bool setResolution(uint32_t xres, uint32_t yres)
{
    writeRegister(SVGA_REG_ENABLE, 0); // disable the output
    writeRegister(SVGA_REG_WIDTH, xres);
    writeRegister(SVGA_REG_HEIGHT, yres);
    writeRegister(SVGA_REG_BPP, 32);   // 32 bits per pixel (4 bytes per pixel as the kernel is using)
    writeRegister(SVGA_REG_ENABLE, 1); // enable it back
    resetFIFO();
    waitFIFO();

    // update the metadata
    fbOffset = readRegister(SVGA_REG_FB_OFFSET);      // read the offset to the guest buffer
    fb->base = (void *)((uint64_t)fbAddr + fbOffset); // set the base address
    fb->currentXres = xres;
    fb->currentYres = yres;

    return true;
}

void forceUpdate()
{
    writeFIFO(SVGA_CMD_UPDATE);
    writeFIFO(0);
    writeFIFO(0);
    writeFIFO(fb->currentXres);
    writeFIFO(fb->currentYres);
    waitFIFO();
}

bool initVMSVGA()
{
    device = (drv_pci_header0_t *)sys_pci_get(SVGA_PCI_VENDOR, SVGA_PCI_DEVICE);

    if (!device) // the device isn't present
        return false;

    device->Command |= 0x7; // enable memory space, io space and bus mastering

    ioBase = device->BAR0 - 1;                   // set the io port base
    fbAddr = device->BAR1;                       // get the framebuffer address
    fifoAddr = device->BAR2;                     // get the fifo address
    fbOffset = readRegister(SVGA_REG_FB_OFFSET); // read the offset to the guest buffer

    printf("vmsvga: ioBase is %x, the fb is at %x and the fifo is at %x\n", ioBase, fbAddr + fbOffset, fifoAddr);

    // check for the highest supported version
    for (; version; version--)
    {
        writeRegister(SVGA_REG_ID, SVGA_GEN_ID(version));      // tell the device that we support the version
        if (SVGA_GEN_ID(version) == readRegister(SVGA_REG_ID)) // check if the device supports it
            break;

        if (!version) // failed to negociate the version
        {
            printf("vmsvga: failed to negociate version\n");
            return false;
        }
    }

    printf("vmsvga: using version %d\n", version);

    // read max resolution
    maxWidth = readRegister(SVGA_REG_MAX_WIDTH);
    maxHeight = readRegister(SVGA_REG_MAX_HEIGHT);

    printf("vmsvga: max resolution %dx%d\n", maxWidth, maxHeight);

    // read sizes
    fifoSize = readRegister(SVGA_REG_FIFO_SIZE);
    fbSize = readRegister(SVGA_REG_FB_SIZE);
    vramSize = readRegister(SVGA_REG_VRAM_SIZE);

    printf("vmsvga: the fifo is %d kb, fb is %d kb and vram is %d kb\n", fifoSize / 1024, fbSize / 1024, vramSize / 1024);

    // map and clear the fifo
    for (int i = 0; i < fifoSize; i += 4096)
        sys_identity_map((void *)(uint64_t)(fifoAddr + i)); // ask the kernel to map the address

    resetFIFO();

    // enable svga mode
    writeRegister(SVGA_REG_ENABLE, 1);

    return true;
}

void _mdrvmain()
{
    fb = (drv_type_framebuffer_t *)sys_drv_announce(SYS_DRIVER_TYPE_FRAMEBUFFER); // announce that we are a framebuffer

    if (!initVMSVGA()) // try to initialise the device
        abort();

    printf("vmsvga: initialised\n");

    setResolution(1024, 768);

    sys_drv_flush(SYS_DRIVER_TYPE_FRAMEBUFFER);

    sys_idt_set(forceUpdate, 0xF00); // update on idle task

    while (1)
        sys_yield(); // don't consume cpu time for nothing
}