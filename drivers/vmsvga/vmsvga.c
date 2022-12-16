#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SVGA_PCI_VENDOR 0x15AD
#define SVGA_PCI_DEVICE 0x0405

drv_type_framebuffer_t *fb;
drv_pci_header0_t *device;

bool detectVMSVGA()
{
    device = (drv_pci_header0_t *)sys_pci_get(SVGA_PCI_VENDOR, SVGA_PCI_DEVICE);
    return device != NULL; // if the adapter is present on the pci bus then it's present
}

void _mdrvmain()
{
    fb = (drv_type_framebuffer_t *)sys_drv_announce(SYS_DRIVER_TYPE_FRAMEBUFFER); // announce that we are a framebuffer

    if (!detectVMSVGA()) // try to detect the device
        abort();

    printf("vmsvga: initialised\n");

    while (1)
        ;
}