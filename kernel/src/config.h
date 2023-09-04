#pragma once

// ======================================================================================
// mOS Kernel configuration
// Uncomment to enable, comment to disable
// Be sure to do `make clean` before rebuilding to be sure the new settings will apply
// ======================================================================================

// ###########################################
// Debug
// ###########################################
#define K_RELEASE // generate release build of the kernel (removes all debug statements);

#ifndef K_RELEASE
#define K_VT_DEBUG      // vt debug messages
#define K_VFS_DEBUG     // vfs debug messages
#define K_SYSCALL_DEBUG // syscall debug messages
#define K_ELF_DEBUG     // elf debug messages
#define K_ACPI_DEBUG    // acpi debug messages
#define K_SCHED_DEBUG   // scheduler debug messages
#define K_VMM_DEBUG     // virtual memory manager debug messages
#define K_PMM_DEBUG     // physical memory manager debug messages
#define K_SOCK_DEBUG    // socket debug messages
#endif

// ###########################################
// CPU
// ###########################################

// SMP
#define K_SMP // enables symetric multiprocessing support (experimental)

#ifdef K_SMP
#define K_MAX_CORES 128 // maximum cores
#else
#define K_MAX_CORES 1 // this decreases kernel memory usage
#endif

// LAPIC
#define K_LAPIC_FREQ 300 // target lapic timer frequency

// ###########################################
// Memory
// ###########################################

// Page frame allocator
// #define K_IGNORE_LOW_MEMORY // ignores memory pools that are in the legacy real mode addressing space (<1 MB addresses, might be necessary on some machines)
// #define K_IGNORE_SMALL_POOLS // ignores small memory pools (<128 kb)

// ###########################################
// Scheduling
// ###########################################

// Scheduler
#define K_SCHED_MIN_QUANTUM 1    // minimum quantum of each task
#define K_SCHED_MAX_ARGUMENTS 32 // maximum arguemnt
#define K_STACK_SIZE 8 * 1024    // userspace stack size

// ###########################################
// Devices
// ###########################################

// =================
// Busses
// =================
#define K_PCIE // enable PCIe ECAM support

// =================
// Console
// =================

// Serial
#define K_COM_ENABLE     // enable support for serial
#define K_COM_BAUD_DIV 1 // divisor of the baud rate (base is 115200)
#define K_COM_LOG        // kernel logger on serial
#define K_COM_WAIT       // wait for serial transmission to be done (has to be enabled on real hardware for reliable serial)

// =================
// Storage
// =================

// --------
// Interfaces
// --------
#define K_AHCI // PCIe AHCI support
#define K_ATA  // ATA/IDE support

// --------
// Filesystems
// --------

// FAT32
#define K_FAT           // enable FAT32 support
#define K_FAT_LOWER_SFN // short file names are always uppercase, force lowering them

// =================
// Display
// =================

// Framebuffer
#define K_FB_SCROLL        // enables framebuffer scrolling (deprecated, to be removed)
#define K_FB_DOUBLE_BUFFER // enables kernel double buffering (decreases flickering at expense of memory and latency)
#define K_FB_CURSOR '_'    // cursor for tty mode
// #define K_FB_TTY_REFRESH_ON_DEMAND // refreshes full screen tty only when changes in text buffer occurs (slight performance improvement on very slow machines)

// ###########################################
// Firmware
// ###########################################

// ACPI
#define K_ACPI_AML // enables experimental AML (ACPI Machine Language) support

// ###########################################
// Miscellaneous
// ###########################################

// Panic
// #define K_PANIC_REBOOT // reboot on kernel panic
// #define K_PANIC_ON_USERSPACE_CRASH // panics on userspace exceptions (useful when debugging drivers and apps)

// Drivers
// #define K_DRIVERS_LOG_TTY // log on tty driver messages (comment to log only on serial)