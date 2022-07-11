#pragma once
#include <utils.h>
#include <idt.h>

uint32_t syscallGetCount();
void syscallHandler(struct idt_intrerrupt_stack *registers);
void syscallInit(uint16_t vector);