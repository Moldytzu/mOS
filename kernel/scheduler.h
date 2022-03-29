#pragma once
#include <utils.h>
#include <idt.h>

void schedulerSchedule(struct idt_intrerrupt_stack *stack);
void schedulerInit();