#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

#define SYSCALL_STATUS_OK 0
#define SYSCALL_STATUS_ERROR 1
#define SYSCALL_STATUS_UNKNOWN_OPERATION 2

void syscallInit();