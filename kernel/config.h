#pragma once

// ==========================================
// mOS Kernel configuration
// Uncomment to enable, comment to disable
// ==========================================

// Debug
//#define K_RELEASE // generate release build of the kernel (removes all debug statements);

#ifndef K_RELEASE
#define K_VT_DEBUG      // vt debug messages; If unsure: comment
#define K_VFS_DEBUG     // vfs debug messages; If unsure: comment
#define K_SYSCALL_DEBUG // syscall debug messages; If unsure: comment
#define K_ELF_DEBUG     // elf debug messages; If unsure: comment
#define K_ACPI_DEBUG    // acpi debug messages; If unsure: comment
#define K_SCHED_DEBUG   // scheduler debug messages; If unsure: comment
#define K_INPUT_DEBUG   // input subsystem debug messages; If unsure: comment
#define K_VMM_DEBUG     // virtual memory manager debug messages; If unsure: comment
#define K_PMM_DEBUG     // physical memory manager debug messages; If unsure: comment
#define K_PS2_DEBUG     // enable ps/2 debug messages; If unsure: comment
#define K_BLDR_DEBUG    // enable bootloader debug messages; If unsure: comment
#define K_SOCK_DEBUG    // enable socket debug messages; If unsure: comment
#endif

// Features
#define K_PS2          // enable ps/2 support; If unsure: uncomment
#define K_IDT_IST      // enable ist; If unsure: uncomment
#define K_IDT_DIS_KIRQ // disable intrerrupts during kernel startup; If unsure: comment