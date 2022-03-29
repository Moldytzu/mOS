#include <scheduler.h>

struct sched_task tasks[0x1000]; // todo: replace this with a linked list
uint16_t lastTID = 0; // last task ID

void schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    // todo: swap the registers and the paging table
}

void schedulerInit()
{
    // initialize the linked list
}