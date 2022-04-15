#pragma once
#include <utils.h>
#include <idt.h>
#include <vmm.h>

#define TASK_STATE_RUNNING 0
#define TASK_STATE_PAUSED 1

#define TASK_BASE_ADDRESS 0xA000000000

struct pack sched_task
{
    char name[128];                              // 128 characters should be enough
    uint8_t priority;                            // higher priority means more ticks allocated to the task
    uint8_t state;                               // current state
    uint16_t id;                                 // task ID (TID)
    struct vmm_page_table *pageTable;            // pointer to the page table
    struct idt_intrerrupt_stack intrerruptStack; // the last intrerrupt stack

    uint8_t priorityCounter; // counter used for priority calculation
};

void schedulerSchedule(struct idt_intrerrupt_stack *stack);
void schedulerInit();
void schedulerEnable();
void schedulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize);
void schedulerPrioritize(uint16_t tid, uint8_t priority);
struct sched_task *schedulerGetCurrent();