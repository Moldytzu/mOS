#pragma once

// ========================================================
// mOS Kernel configuration
// Uncomment to enable, comment to disable
// Be sure to perform a full rebuild when changing settings
// ========================================================

// Debug
#define K_RELEASE // generate release build of the kernel (removes all debug statements);

#ifndef K_RELEASE
#define K_VT_DEBUG      // vt debug messages
#define K_VFS_DEBUG     // vfs debug messages
#define K_SYSCALL_DEBUG // syscall debug messages
#define K_ELF_DEBUG     // elf debug messages
#define K_ACPI_DEBUG    // acpi debug messages
#define K_SCHED_DEBUG   // scheduler debug messages
#define K_INPUT_DEBUG   // input subsystem debug messages
#define K_VMM_DEBUG     // virtual memory manager debug messages
#define K_PMM_DEBUG     // physical memory manager debug messages
#define K_HEAP_DEBUG    // heap debug messages
#define K_BLDR_DEBUG    // bootloader debug messages
#define K_SOCK_DEBUG    // socket debug messages
#endif

// Constants
#define K_STACK_SIZE 64 * 1024 // userspace stack size
#define K_MAX_CORES 128        // maximum cores

// Serial
#define K_COM_ENABLE     // enable support for serial
#define K_COM_BAUD_DIV 1 // divisor of the baud rate (base is 115200)

// PIT
#define K_PIT_FREQ 200 // frequency in hz of the pit

// IDT
#define K_IDT_IST // enable ist

// Panic
// #define K_PANIC_REBOOT // reboot on kernel panic