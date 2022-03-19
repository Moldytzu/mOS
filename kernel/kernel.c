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

    // display the memory available
    framebufferWrite("Memory available for the kernel ");
    framebufferWrite(to_string(toMB(mmGetInfo().available)));
    framebufferWrite(" MB.\n");

    // display the pools
    struct mm_pool *pools = mmGetPools();
    for(int i = 0; pools[i].allocableBase; i++)
    {
        framebufferWrite("Total: ");
        framebufferWrite(to_string(pools[i].total));
        framebufferWrite(" Available: ");
        framebufferWrite(to_string(pools[i].total));
        framebufferWrite("\n");
    }

    // allocate some memory
    for(int i = 0; i < 16; i++)
    {
        framebufferWrite(to_string((uint64_t)mmAllocatePage()));
        framebufferWrite("\n");
    }

    // hang
    while (1)
        asm volatile("hlt");
}
