#include <scheduler.h>
#include <pmm.h>
#include <gdt.h>

void *kernelStack;
struct sched_task tasks[0x1000]; // todo: replace this with a linked list
uint16_t lastTID = 0;            // last task ID
bool enabled = false;            // enabled

void schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    if (!enabled)
        return; // don't do anything if it isn't enabled
    // todo: swap the registers and the paging table
}

void schedulerInit()
{
    memset64(tasks, 0, 0x1000 * sizeof(struct sched_task) / sizeof(uint64_t)); // clear the tasks
    kernelStack = mmAllocatePage();                                            // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack;                                  // set kernel stack in tss
}

void schedulerEnable()
{
    enabled = true; // enable
}

void schdulerAdd(const char *name, void *entry, uint64_t stackSize)
{
    void *stack = mmAllocatePages(stackSize / 4096); // allocate stack for the task

    uint16_t index = ++lastTID;
    tasks[index].tid = index;                              // set the task ID
    memcpy(tasks[index].name, (char *)name, strlen(name)); // set the name

    // initial registers
    tasks[index].intrerruptStack.rip = (uint64_t)entry;               // set the entry point a.k.a the instruction pointer
    tasks[index].intrerruptStack.rflags = 0x202;                      // rflags, enable intrerrupts
    tasks[index].intrerruptStack.krsp = (uint64_t)kernelStack + 4096; // kernel stack
    tasks[index].intrerruptStack.rsp = (uint64_t)stack + stackSize;   // task stack
    tasks[index].intrerruptStack.cs = 8 * 3;                          // code segment for userspace is the 3rd
    tasks[index].intrerruptStack.ss = 8 * 4;                          // data segment for userspace is the 4th

    // page table
    struct vmm_page_table *newTable = mmAllocatePage();             // allocate a new page table
    memcpy64(newTable, vmmGetBaseTable(), 4096 / sizeof(uint64_t)); // copy the base page table over our new table
    tasks[index].pageTable = newTable;                              // set the new page table

    // todo: map the executable
}