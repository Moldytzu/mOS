#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>
#include <framebuffer.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    bootloaderInit(stivale2_struct); // init bootloader

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
    GDTInit();
    framebufferWrite("done\n");

    // initialize the idt
    framebufferWrite("Initializing the IDT...");
    IDTInit();
    framebufferWrite("done\n");

    // cause an intrerrupt
    asm volatile ("int $0");

    // hang
    for (;;) {
        asm volatile ("hlt");
    }
}
