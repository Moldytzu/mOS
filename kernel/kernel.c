#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>
#include <framebuffer.h>
#include <fpu.h>
#include <mm.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    // initialize the fpu
    fpuInit();

    // initialize the bootloader interface
    bootloaderInit(stivale2_struct);

    // initialize framebuffer
    framebufferInit();

    // display message
    framebufferWrite("Starting up the kernel.\n");

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

    // initialize the memory manager
    framebufferWrite("Initializing the Memory Manager...");
    mmInit();
    framebufferWrite("done\n");

    framebufferWrite("Memory available for the kernel ");
    framebufferWrite(to_string(toMB(mmGetInfo().total)));
    framebufferWrite(" MB\n");

    for(int i = 0; i < 16; i++) // allocate 16 pages
    {
        framebufferWrite(to_string((uint64_t)mmAllocatePage()));
        framebufferWrite("\n");
    }

    // cause an intrerrupt
    asm volatile ("int $0");

    // hang
    for (;;) {
        asm volatile ("hlt");
    }
}
