#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <mm/vma.h>
#include <ipc/mailbox.h>

#define TASK_BASE_SWITCH_TO_BUFFER 0x1000  // this has to be hardcoded
#define TASK_BASE_FRAMEBUFFER 0xC000000000 // fixme: replace this with allocated virtual addresses
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
    uint8_t targetQuantum;

    mailbox_t mailbox;

    // context
    uint8_t align_addr(16) simdContext[512];
    idt_intrerrupt_stack_t align_addr(16) registers;
    vmm_page_table_t *pageTable;
    char *enviroment;

    // internal
    vma_context_t *virtualMemoryContext;
    uint64_t fileDescriptorPointers[TASK_MAX_FILE_DESCRIPTORS];
    void *stackBase;
    void *elfBase;
    size_t elfSize;
    uint64_t virtualBaseAddress;
    uint32_t quantumLeft;
    void **allocated;              // allocated pages
    uint32_t allocatedIndex;       // current index
    uint32_t allocatedBufferPages; // pages used by the buffer

    void *next;
    void *prev;
} sched_task_t;

void schedEnable();
sched_task_t *schedAdd(const char *name, void *entry, void *execPhysicalBase, uint64_t execSize, uint64_t execVirtualBase, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver);
void schedSchedule(idt_intrerrupt_stack_t *stack);
void schedSwitchNext();
void schedInit();
sched_task_t *schedGetCurrent(uint32_t core);
sched_task_t *schedGet(uint32_t id);
void schedKill(uint32_t id);