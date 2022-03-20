#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>
#include <framebuffer.h>
#include <fpu.h>
#include <mm.h>
#include <pic.h>
#include <pit.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct)
{
    // initialize the fpu
    fpuInit();

    // initialize the bootloader interface
    bootloaderInit(stivale2_struct);

    // initialize framebuffer
    framebufferInit();

    // display message
    framebufferWrite("Starting up mOS' kernel in ");
    if(bootloaderGetFirmwareType()) framebufferWrite("BIOS");
    else framebufferWrite("UEFI");
    framebufferWrite(" mode.\n");

    // display framebuffer information
    framebufferWrite("Got framebuffer with the size ");
    framebufferWrite(to_string(bootloaderGetFramebuf()->framebuffer_width));
    framebufferWrite("x");
    framebufferWrite(to_string(bootloaderGetFramebuf()->framebuffer_height));
    framebufferWrite(".\n");

    // initialize the gdt
    framebufferWrite("Initializing the GDT...");
    gdtInit();
    framebufferWrite("done\n");

    // initialize the idt
    framebufferWrite("Initializing the IDT...");
    idtInit();
    framebufferWrite("done\n");

    // initialize the pic chips
    framebufferWrite("Initializing the PICs...");
    picInit();
    framebufferWrite("done\n");

    // initialize the timer
    framebufferWrite("Initializing the PIT...");
    pitInit();
    framebufferWrite("done\n");

    // initialize the memory manager
    framebufferWrite("Initializing the Memory Manager...");
    mmInit();
    framebufferWrite("done\n");

    // get the pools
    struct mm_pool *pools = mmGetPools();
    size_t available = 0;
    for(int i = 0; pools[i].total != UINT64_MAX; i++)
        available += pools[i].available;

    // display the memory available
    framebufferWrite("Memory available for the kernel ");
    framebufferWrite(to_string(toMB(available)));
    framebufferWrite(" MB.\n");

    // allocate some memory
    for(int i = 0; i < 256; i++)
    {
        framebufferWrite(to_hstring((uint64_t)mmAllocatePage()));
        framebufferWrite(" ");
    }

    // hang
    while (1)
        iasm("hlt");
}
