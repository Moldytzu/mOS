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

extern void idleTask();

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

    // initialize the physical memory manager
    printk("Initializing the Physical Memory Manager...");
    pmmInit();
    printk("done\n");

    // initialize the virtual memory manager
    printk("Initializing the Virtual Memory Manager...");
    vmmInit();
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

    // initialize the scheduler
    printk("Initializing the Scheduler...");
    schedulerInit();
    printk("done\n");

    // initialize system calls
    syscallInit(0x51);

    // get the pool total values
    struct mm_pool total = mmGetTotal();

    // display the memory available
    printk("Memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved), total.pageIndex);

    void *task = mmAllocatePage(); // create an empty page just for the idle task
    memcpy(task,(void*)idleTask,4096); // copy it

    schdulerAdd("Idle Task", task, 4096, task, 4096); // create the idle task
    schdulerAdd("Idle Task 2", task, 4096, task, 4096); // create the idle task
    schedulerEnable(); // enable the schduler

    ((void(*)())task)(); // run the idle task
}

void panick(const char *msg)
{
    printk("\n\nA kernel exception happened.\n%s\n",msg);
    hang();
}