#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <drv/serial.h>
#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <drv/input.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <mm/blk.h>
#include <sched/pit.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <subsys/socket.h>
#include <main/panic.h>

void kmain();

// entry point of the kernel
void _start()
{
    fpuInit(); // initialize the fpu

    bootloaderInit(); // initialize the bootloader interface

    initrdInit(); // initialize the initrd

    framebufferInit(); // initialize framebuffer

    serialInit(); // initialize the serial port

    pmmInit(); // initialize the physical memory manager

    gdtInit(); // initialize the gdt

    idtInit(); // initialize the idt

    vmmInit(); // initialize the virtual memory manager

    blkInit(); // initialize the block allocator

    heapInit(); // initialize the heap

    vfsInit(); // initialize the virtual filesystem

    initrdMount(); // mount the initrd

    picInit(); // initialize the pic chips

    pitInit(); // initialize the timer

    schedulerInit(); // initialize the scheduler

    acpiInit(); // initialize the acpi interface

    inputInit(); // initialize the input subsystem

    sockInit(); // initialize the ipc (sockets)

    syscallInit(0x51); // initialize system calls

    kmain(); // call main

    __builtin_unreachable(); // can't reach this
}