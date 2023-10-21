#include <misc/utils.h>
#include <misc/logger.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/smp.h>
#include <cpu/control.h>
#include <cpu/xapic.h>
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
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fs/initrd.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <ipc/socket.h>
#include <main/panic.h>

void kmain();

// entry point of the kernel
void _start()
{
    fpuInit(); // initialise the fpu

    serialInit(); // initialise the serial port

    framebufferInit(); // initialise framebuffer

    logInfo("cmdline: %s", bootloaderGetCommandLine());

    // initrdInit(); // initialise the initrd

    pmmInit(); // initialise the physical memory manager

    smpBootstrap(); // bootstrap the cpus

    blkInit(); // initialise the block allocator

    acpiInit(); // initialise the acpi interface

    picInit(); // initialise the pic chips

    xapicInit(true); // initalize the advanced intrerupt controller

    ioapicInit(); // initialise the external interrupt controller

    acpiInstallSCIInterrupt(); // install SCI interrupt handler

    vfsInit(); // initialise the virtual filesystem

    // initrdMount(); // mount the initrd

#ifdef K_ATA
    // ataInit(); // initialise ide/ata pio hard drives
#endif

#ifdef K_AHCI
    // ahciInit(); // initialise ahci controller
#endif

    schedInit(); // create initial tasks

    drvInit(); // initialise the driver manager

    inputInit(); // initialise the input subsystem

    sockInit(); // initialise the ipc (sockets)

    syscallInit(); // initialise system calls

    kmain(); // call main

    __builtin_unreachable(); // can't reach this
}