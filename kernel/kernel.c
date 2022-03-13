#include <utils.h>
#include <bootloader.h>
#include <idt.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    bootloaderInit(stivale2_struct); // init bootloader
    bootloaderTermWrite("Starting up the kernel.\n"); // display a message
    
    // initialize the idt
    bootloaderTermWrite("Initializing the IDT...");
    IDTInit();
    bootloaderTermWrite("done\n");

    // test idt
    int a = 5;
    int b = 0;
    int c = a/b;

    // hang
    for (;;) {
        asm volatile ("hlt");
    }
}
