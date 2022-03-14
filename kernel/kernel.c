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

    // initialize the gdt
    SerialWrite("Initializing the GDT...");
    GDTInit();
    SerialWrite("done\n");

    // initialize the idt
    SerialWrite("Initializing the IDT...");
    IDTInit();
    SerialWrite("done\n");

    // cause an intrerrupt
    asm volatile ("int $0");

    // hang
    for (;;) {
        asm volatile ("hlt");
    }
}
