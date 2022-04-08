#pragma once

// ==========================================
// mOS Kernel configuration
// Uncomment to enable, comment do disable
// ==========================================

#define K_SCHED_DEBUG // scheduler debug messages; If unsure: comment
#define K_VMM_DEBUG   // virtual memory manager debug messages; If unsure: comment
#define K_PMM_DEBUG   // physical memory manager debug messages; If unsure: commnet
#define K_IDT_IST     // enable ist; If unsure: uncomment