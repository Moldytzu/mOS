#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <ipc/mailbox.h>

#define TASK_BASE_ADDRESS 0xA000000000
#define TASK_BASE_ALLOC 0xB000000000
#define TASK_BASE_FRAMEBUFFER 0xC000000000
#define TASK_MAX_FILE_DESCRIPTORS 128

typedef struct
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

    mailbox_t mailbox;

    // context
    uint8_t simdContext[512];
    idt_intrerrupt_stack_t registers;
    vmm_page_table_t *pageTable;
    uint64_t lastVirtualAddress;
    char *enviroment;

    // internal
    uint64_t fileDescriptorPointers[TASK_MAX_FILE_DESCRIPTORS];
    void *stackBase;
    void *elfBase;
    size_t elfSize;
    uint32_t quantumLeft;
    void **allocated;              // allocated pages
    uint32_t allocatedIndex;       // current index
    uint32_t allocatedBufferPages; // pages used by the buffer

    void *next;
    void *prev;
} sched_task_t;

void schedEnable();
sched_task_t *schedAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver);
void schedSchedule(idt_intrerrupt_stack_t *stack);
void schedSwitchNext();
void schedInit();
sched_task_t *schedGetCurrent(uint32_t core);
sched_task_t *schedGet(uint32_t id);
void schedKill(uint32_t id);