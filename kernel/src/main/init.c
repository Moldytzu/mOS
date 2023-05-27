#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/smp.h>
#include <cpu/control.h>
#include <cpu/lapic.h>
#include <cpu/ioapic.h>
#include <drv/serial.h>
#include <drv/framebuffer.h>
#include <drv/ahci.h>
#include <drv/ata.h>
#include <drv/input.h>
#include <mm/pmm.h>
#include <mm/blk.h>
#include <drv/drv.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <sched/hpet.h>
#include <sched/time.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fs/initrd.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <subsys/socket.h>
#include <main/panic.h>

void kmain();

// entry point of the kernel
void _start()
{
    fpuInit(); // initialise the fpu

    framebufferInit(); // initialise framebuffer

    initrdInit(); // initialise the initrd

    serialInit(); // initialise the serial port

    pmmInit(); // initialise the physical memory manager

    blkInit(); // initialise the block allocator

    smpBootstrap(); // bootstrap the cpus

    acpiInit(); // initialise the acpi interface

    hpetInit(); // initialise the hpet

    timeSource(); // switch to best time source

    picInit(); // initialise the pic chips

    lapicInit(true); // initalize the advanced intrerupt controller

    ioapicInit(); // initialise the external interrupt controller

    vfsInit(); // initialise the virtual filesystem

    initrdMount(); // mount the initrd

    ataInit(); // initialise ide/ata pio hard drives

    ahciInit(); // initialise ahci controller

    schedInit(); // create initial tasks

    drvInit(); // initialise the driver manager

    inputInit(); // initialise the input subsystem

    sockInit(); // initialise the ipc (sockets)

    syscallInit(); // initialise system calls

    kmain(); // call main

    __builtin_unreachable(); // can't reach this
}