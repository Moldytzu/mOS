#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <drv/serial.h>
#include <drv/ps2.h>
#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/pit.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <subsys/input.h>
#include <subsys/socket.h>
#include <main/panic.h>

void kmain();

// entry point of the kernel
void _start()
{
    // initialize the fpu
    fpuInit();

    // initialize the bootloader interface
    bootloaderInit();

    // initialize the initrd
    initrdInit();

    // initialize framebuffer
    framebufferInit();

    // display message
    printk("Starting up mOS' kernel\n");

    // test for the required features
    if (!fpuCheck())
        panick("Unsupported CPU. SSE 4.2 isn't supported!");

    // display framebuffer information
    printk("Got framebuffer with the size %dx%d.\n", bootloaderGetFramebuffer()->width, bootloaderGetFramebuffer()->height);

    // initialize the serial port
    printk("Initializing the Serial Port COM1...");
    serialInit();
    printk("done\n");

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

    // initialize the virtual filesystem
    printk("Initializing the Virtual Filesystem...");
    vfsInit();
    initrdMount(); // mount the initrd
    printk("done\n");

    // initialize the pic chips
    printk("Initializing the PICs...");
    picInit();
    printk("done\n");

    // initialize the timer
    printk("Initializing the PIT...");
    pitInit();
    printk("done\n");

    // initialize the scheduler
    printk("Initializing the Scheduler...");
    schedulerInit();
    printk("done\n");

    // initialize the acpi interface
    printk("Initializing the ACPI interface...");
    acpiInit();
    printk("done\n");

    // initialize the input subsystem
    printk("Initializing the Input subystem...");
    inputInit();
    printk("done\n");

    // initialize the ps2 controller and devices if enabled
#ifdef K_PS2
    printk("Initializing the PS/2 devices...");
    ps2Init();
    printk("done\n");
#endif

    // initialize the ipc (sockets)
    printk("Initializing IPC...");
    sockInit();
    printk("done\n");

    // initialize system calls
    syscallInit(0x51);

    printk("Starting phase ended. Jumping in userspace.\n");

    kmain(); // call main

    printk("Jump out of kmain.");
}