#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

pstruct
{
    uint32_t id;
    idt_intrerrupt_stack_t registers;

    void *next;
    void *prev;
}
sched_task_t;

void schedEnable();
void schedSchedule(idt_intrerrupt_stack_t *stack);
void schedInit();