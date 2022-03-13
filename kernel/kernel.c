#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    bootloaderInit(stivale2_struct); // init bootloader
    bootloaderTermWrite("Starting up the kernel. Switching to serial console display\n"); // display a message

    // initialize the gdt
    SerialWrite("Initializing the GDT...");
    GDTInit();
    SerialWrite("done\n");

    // initialize the idt
    SerialWrite("Initializing the IDT...");
    IDTInit();
    SerialWrite("done\n");

    // test idt by issuing a #DE
    int a = 5;
    int b = 0;
    int c = a/b;

    // hang
    for (;;) {
        asm volatile ("hlt");
    }
}
