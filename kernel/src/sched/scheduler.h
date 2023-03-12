#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>

#define TASK_BASE_ADDRESS 0xA000000000
#define TASK_BASE_ALLOC 0xB000000000

pstruct
{
    // metadata
    uint32_t core;
    uint32_t id;
    uint32_t terminal;
    uint8_t state;
    char name[128];
    char cwd[512];
    bool isDriver;
    bool isElf;

    // context
    idt_intrerrupt_stack_t registers;
    uint64_t pageTable;
    uint64_t lastVirtualAddress;
    char *enviroment;

    // internal
    uint32_t quantumLeft;
    void **allocated;              // allocated pages
    uint32_t allocatedIndex;       // current index
    uint32_t allocatedBufferPages; // pages used by the buffer

    void *next;
    void *prev;
}
sched_task_t;

void schedEnable();
sched_task_t *schedAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver);
void schedSchedule(idt_intrerrupt_stack_t *stack);
void schedInit();
sched_task_t *schedGetCurrent(uint32_t core);
sched_task_t *schedGet(uint32_t id);
void schedKill(uint32_t id);