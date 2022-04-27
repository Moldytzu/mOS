#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>
#include <framebuffer.h>
#include <fpu.h>
#include <pmm.h>
#include <vmm.h>
#include <pic.h>
#include <pit.h>
#include <syscall.h>
#include <scheduler.h>
#include <control.h>
#include <heap.h>
#include <acpi.h>
#include <initrd.h>

void kmain();

// entry point of the kernel
void _start(struct stivale2_struct *stivale2_struct)
{
    // initialize the fpu
    fpuInit();

    // load a custom cr4
    controlLoadCR4(0b0000000000000001000100000); // enable PAE and OSFXSR

    // initialize the bootloader interface
    bootloaderInit(stivale2_struct);

    // initialize the initrd
    initrdInit();

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

    // initialize the physical memory manager
    printk("Initializing the Physical Memory Manager...");
    pmmInit();
    printk("done\n");

    // initialize the gdt
    printk("Initializing the GDT...");
    gdtInit();
    printk("done\n");

    // initialize the idt
    printk("Initializing the IDT...");
    idtInit();
    printk("done\n");

    // initialize the virtual memory manager
    printk("Initializing the Virtual Memory Manager...");
    vmmInit();
    printk("done\n");

    // initialize the heap
    printk("Initializing the Heap...");
    heapInit();
    printk("done\n");

    // initialize the pic chips
    printk("Initializing the PICs...");
    picInit();
    printk("done\n");

    // initialize the timer
    printk("Initializing the PIT...");
    pitInit();
    printk("done\n");

    // initialize the acpi interface
    printk("Initializing the ACPI interface...");
    acpiInit();
    printk("done\n");

    // initialize the scheduler
    printk("Initializing the Scheduler...");
    schedulerInit();
    printk("done\n");

    // initialize system calls
    syscallInit(0x51);

    printk("Starting phase ended. Jumping in userspace.\n");

    kmain(); // call main

    printk("Jump out of kmain.");
}