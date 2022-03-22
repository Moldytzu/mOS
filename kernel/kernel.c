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
    printk("Starting up mOS' kernel in ");
    if (bootloaderGetFirmwareType())
        printk("BIOS");
    else
        printk("UEFI");
    printk(" mode.\n");

    // display framebuffer information
    printk("Got framebuffer with the size %dx%d.\n", bootloaderGetFramebuf()->framebuffer_width, bootloaderGetFramebuf()->framebuffer_height);

    // initialize the memory manager
    printk("Initializing the Memory Manager...");
    mmInit();
    printk("done\n");

    // initialize the gdt
    printk("Initializing the GDT...");
    gdtInit();
    printk("done\n");

    // initialize the idt
    printk("Initializing the IDT...");
    idtInit();
    printk("done\n");

    // initialize the pic chips
    printk("Initializing the PICs...");
    picInit();
    printk("done\n");

    // initialize the timer
    printk("Initializing the PIT...");
    pitInit();
    printk("done\n");

    // get the pool total values
    struct mm_pool total = mmGetTotal();

    // display the memory available
    printk("Memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB;\n", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved));

    // hang
    while (1)
        iasm("hlt");
}
