#pragma once
#include <utils.h>
#include <idt.h>
#include <vmm.h>

#define TASK_STATE_RUNNING 0
#define TASK_STATE_PAUSED 1

struct pack sched_task
{
    char name[128];                              // 127 characters should be enough
    uint8_t state;                               // current state
    uint16_t tid;                                // task ID
    struct vmm_page_table *pageTable;            // pointer to the page table
    struct idt_intrerrupt_stack *intrerruptStack; // the last intrerrupt stack
};

void schedulerSchedule(struct idt_intrerrupt_stack *stack);
void schedulerInit();
void schedulerEnable();
void schdulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize);