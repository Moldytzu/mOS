#pragma once
#include <misc/utils.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <subsys/vt.h>

#define TASK_STATE_RUNNING 0
#define TASK_STATE_PAUSED 1

#define TASK_BASE_ADDRESS 0xA000000000
#define TASK_BASE_ALLOC 0xB000000000

struct sched_task
{
    char name[512];                         // name
    char cwd[512];                          // current working directory
    uint8_t priority;                       // higher priority means more ticks allocated to the task
    uint8_t priorityCounter;                // counter used for priority calculation
    uint8_t state;                          // current state
    uint32_t id;                            // task ID (TID)
    vmm_page_table_t *pageTable;            // pointer to the page table
    idt_intrerrupt_stack_t intrerruptStack; // the last intrerrupt stack
    uint8_t simdContext[512];               // simd context (contains sse and fpu information)
    uint32_t terminal;                      // task terminal
    void **allocated;                       // allocated pages
    uint32_t allocatedIndex;                // current index
    void *lastVirtualAddress;               // last virtual address of the allocation
    char *enviroment;                       // enviroment variables
    uint32_t syscallUsage;                  // the count of syscalls issued by the task in a cycle
    uint8_t overallCPUpercent;              // percent of the cpu used
    uint32_t sleep;                         // ticks to sleep
    bool elf;                               // is elf
    uint64_t elfSize;                       // size of the executable
    void *elfBase;                          // base of the executable
    void *stackBase;                        // base of the stack
    uint64_t stackSize;                     // size of the stack
    bool isDriver;                          // is driver

    struct sched_task *previous; // previous task
    struct sched_task *next;     // next task
};

void schedulerSchedule(idt_intrerrupt_stack_t *stack);
void schedulerInit();
void schedulerSkipNextSaving();
bool schedulerEnabled();
void schedulerEnable();
void schedulerPrioritize(uint32_t tid, uint8_t priority);
void schedulerSetTerminal(uint32_t tid, uint32_t terminal);
void schedulerKill(uint32_t tid);
void schedulerHandleDriver(void *handler, uint32_t tid);
uint32_t schedulerGetLastID();
struct sched_task *schedulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver);
struct sched_task *schedulerGet(uint32_t tid);
struct sched_task *schedulerGetCurrent();