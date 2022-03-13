#include <utils.h>
#include <bootloader.h>

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct) {
    bootloaderInit(stivale2_struct); // init bootloader

    // display something
    bootloaderTermWrite("Hello world!");

    // We're done, just hang...
    for (;;) {
        asm ("hlt");
    }
}
