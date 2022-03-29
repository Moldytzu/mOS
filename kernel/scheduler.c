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

    vmmSwap(tasks[0].pageTable);                                                   // load the page table
    memcpy(stack, &tasks[0].intrerruptStack, sizeof(struct idt_intrerrupt_stack)); // copy the registers

    printk("new: cs=%d ss=%d rip=%p", stack->cs, stack->ss, stack->rip);
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

void schdulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize)
{
    uint16_t index = lastTID++;
    tasks[index].tid = index;                              // set the task ID
    memcpy(tasks[index].name, (char *)name, strlen(name)); // set the name

    // page table
    struct vmm_page_table *newTable = mmAllocatePages(4); // allocate a new page table
    tasks[index].pageTable = newTable;                    // set the new page table

    void *stack = mmAllocatePages(stackSize / 4096); // allocate stack for the task

    for (size_t i = 0; i < stackSize; i += 4096)     // map stack as user, read-write
        vmmMap(newTable, stack + i, stack + i, true, true);

    for (size_t i = 0; i < execSize; i += 4096) // map executable as user, read-write
        vmmMap(newTable, execBase + i, execBase + i, true, true);

    vmmMap(newTable, kernelStack, kernelStack, false, true); // map kernel stack as read-write

    // initial registers
    tasks[index].intrerruptStack.rip = (uint64_t)entry;               // set the entry point a.k.a the instruction pointer
    tasks[index].intrerruptStack.rflags = 0x002;                      // rflags, disable intrerrupts
    tasks[index].intrerruptStack.krsp = (uint64_t)kernelStack + 4096; // kernel stack
    tasks[index].intrerruptStack.rsp = (uint64_t)stack + stackSize;   // task stack
    tasks[index].intrerruptStack.cs = 8 * 1;                          // code segment for kernelspace is the 3rd
    tasks[index].intrerruptStack.ss = 8 * 2;                          // data segment for kernelspace is the 4th
}
