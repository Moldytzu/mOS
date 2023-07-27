#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// ids
#define BGA_PCI_VENDOR 0x1234
#define BGA_PCI_DEVICE 0x1111

#define VBOX_PCI_VENDOR 0x80EE
#define VBOX_PCI_DEVICE 0xBEEF

// ports
#define BGA_INDEX 0x1CE
#define BGA_DATA 0x1CF

// registers
#define BGA_REG_ID 0
#define BGA_REG_XRES 1
#define BGA_REG_YRES 2
#define BGA_REG_BPP 3
#define BGA_REG_VBE_ENABLE 4
#define BGA_REG_BANK 5
#define BGA_REG_VIRT_WIDTH 6
#define BGA_REG_VIRT_HEIGHT 7
#define BGA_REG_X_OFFSET 8
#define BGA_REG_Y_OFFSET 9

// commands
#define VBE_DISABLED 0x00
#define VBE_ENABLED 0x01
#define VBE_GETCAPS 0x02
#define VBE_BANK_GRANULARITY_32K 0x10
#define VBE_8BIT_DAC 0x20
#define VBE_LFB_ENABLED 0x40
#define VBE_NOCLEARMEM 0x80

// constants
#define BGA_MAX_XRES 1280
#define BGA_MAX_YRES 768

typedef struct
{
    uint32_t x, y;
} bga_resolution_t;

drv_type_framebuffer_t *fb;
drv_pci_header0_t *device;

uint16_t readRegister(uint16_t reg)
{
    outw(BGA_INDEX, reg); // point to the register
    return inw(BGA_DATA); // read the contents
}

void writeRegister(uint16_t reg, uint16_t data)
{
    outw(BGA_INDEX, reg); // point to the register
    outw(BGA_DATA, data); // write the data
}

void disableVBE()
{
    writeRegister(BGA_REG_VBE_ENABLE, VBE_DISABLED); // disable vbe
}

void enableVBE()
{
    writeRegister(BGA_REG_VBE_ENABLE, VBE_ENABLED | VBE_LFB_ENABLED | VBE_NOCLEARMEM); // enable vbe and the linear framebuffer
}

bool detectBGA()
{
    // search for qemu/bochs device
    device = (drv_pci_header0_t *)sys_pci_get(BGA_PCI_VENDOR, BGA_PCI_DEVICE);

    // search for vbox device
    if (!device)
        device = (drv_pci_header0_t *)sys_pci_get(VBOX_PCI_VENDOR, VBOX_PCI_DEVICE);

    // nah
    if (!device)
        return false;

    // detect version
    uint32_t version = readRegister(BGA_REG_ID);

    printf("bga: detected version %d", version - 0xB0C0);

    return version >= 0xB0C2; // we want a new version
}

bool setResolution(uint32_t xres, uint32_t yres)
{
    // invalid resolution
    if ((xres == 0 || yres == 0) || (xres % 8 != 0 || yres % 8 != 0) || (xres > BGA_MAX_XRES || yres > BGA_MAX_YRES))
        return false;

    disableVBE();
    writeRegister(BGA_REG_XRES, xres); // set horizontal resolution
    writeRegister(BGA_REG_YRES, yres); // set vertical resolution
    writeRegister(BGA_REG_BPP, 32);    // set 32 bits per pixel
    enableVBE();                       // enable vbe

    // update the metadata
    fb->base = (void *)(uint64_t)device->BAR0; // set the base address

    fb->currentXres = xres;
    fb->currentYres = yres;

    return true;
}

bga_resolution_t getResolution()
{
    bga_resolution_t res;
    disableVBE();
    res.x = readRegister(BGA_REG_XRES);
    res.y = readRegister(BGA_REG_YRES);
    enableVBE(); // enable vbe

    return res;
}

void _mdrvmain()
{
    if (!detectBGA())
    {
        puts("bga: failed to initialise!\n");
        abort();
    }

    fb = (drv_type_framebuffer_t *)sys_driver_announce(SYS_DRIVER_TYPE_FRAMEBUFFER); // announce that we are a framebuffer

    if (!fb)
    {
        puts("bga: failed to announce!\n");
        abort();
    }

    setResolution(640, 480); // set a default resolution

    sys_driver_flush(SYS_DRIVER_TYPE_FRAMEBUFFER); // flush the changes

    puts("bga: initialised\n");

    while (1)
    {
        for (int i = 0; i < 10; i++) // don't waste time here by yielding
            sys_yield();

        // wait for the kernel to request another resolution
        if (fb->requestedXres && fb->requestedXres && fb->currentXres == fb->requestedXres && fb->currentYres == fb->requestedYres)
            continue;

        if (!setResolution(fb->requestedXres, fb->requestedYres)) // try to change the resolution
        {
            printf("bga: failed to set %dx%d\n", fb->requestedXres, fb->requestedYres);
            fb->requestedXres = fb->currentXres;
            fb->requestedYres = fb->currentYres;
        }

        sys_yield();

        sys_driver_flush(SYS_DRIVER_TYPE_FRAMEBUFFER); // flush the changes

        printf("bga: new resolution %dx%d\n", fb->currentXres, fb->currentYres);
    }
}