#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

uint32_t syscallGetCount();
void syscallHandler(idt_intrerrupt_stack_t *registers);
void syscallInit();