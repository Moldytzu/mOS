#include <scheduler.h>
#include <pmm.h>
#include <gdt.h>
#include <bootloader.h>
#include <serial.h>

void *kernelStack;
struct sched_task tasks[0x1000];      // todo: replace this with a linked list
uint16_t lastTID = 0, currentTID = 0; // last task ID, current task ID
bool enabled = false;                 // enabled

extern void userspaceJump(uint64_t rip, uint64_t stack);

void schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    if (!enabled)
        return; // don't do anything if it isn't enabled

    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SCHED_DEBUG
    serialWrite("sched: saving ");
    serialWrite(tasks[currentTID].name);
    serialWrite("\n");
#endif

    // save the registers
    memcpy(&tasks[currentTID].intrerruptStack, stack, sizeof(struct idt_intrerrupt_stack));

    // load the next task
    currentTID++;
    if (currentTID == lastTID)
        currentTID = 0; // reset tid if we're overrunning

#ifdef K_SCHED_DEBUG
    serialWrite("sched: loading ");
    serialWrite(tasks[currentTID].name);
    serialWrite("\n");
#endif

    uint64_t krsp = stack->krsp; // save the stack
    memcpy(stack, &tasks[currentTID].intrerruptStack, sizeof(struct idt_intrerrupt_stack));
    stack->krsp = krsp;                       // restore it
    tssGet()->rsp[0] = (uint64_t)stack->krsp; // set kernel stack in tss

    vmmSwap(tasks[currentTID].pageTable); // swap the page table
}

void schedulerInit()
{
    memset64(tasks, 0, 0x1000 * sizeof(struct sched_task) / sizeof(uint64_t)); // clear the tasks
    kernelStack = mmAllocatePage();                                            // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack + VMM_PAGE;                       // set kernel stack in tss
}

void schedulerEnable()
{
    vmmSwap(tasks[currentTID].pageTable); // swap the page table
    enabled = true; // enable the scheduler
    userspaceJump(0xA000000000,tasks[currentTID].intrerruptStack.rsp); // jump in userspace
}

void schedulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize)
{
    uint16_t index = lastTID++;
    tasks[index].tid = index;                              // set the task ID
    memcpy(tasks[index].name, (char *)name, strlen(name)); // set the name

    // page table
    struct vmm_page_table *newTable = vmmCreateTable(false); // create a new page table
    tasks[index].pageTable = newTable;                       // set the new page table

    void *stack = mmAllocatePages(stackSize / VMM_PAGE); // allocate stack for the task

    for (size_t i = 0; i < stackSize; i += VMM_PAGE) // map task stack as user, read-write
        vmmMap(newTable, (void *)stack + i, stack + i, true, true);

    for (size_t i = 0; i < execSize; i += VMM_PAGE)
        vmmMap(newTable, (void *)0xA000000000 + i, (void *)execBase + i, true, true); // map task as user, read-write

    // initial registers
    tasks[index].intrerruptStack.rip = 0xA000000000 + (uint64_t)entry; // set the entry point a.k.a the instruction pointer
    tasks[index].intrerruptStack.rflags = 0x202;                       // rflags, enable intrerrupts
    tasks[index].intrerruptStack.rsp = (uint64_t)stack + stackSize;    // task stack
    tasks[index].intrerruptStack.cs = 8 * 1;                           // code segment for kernel is the first
    tasks[index].intrerruptStack.ss = 8 * 2;                           // data segment for kernel is the second
}

struct sched_task *schedulerGetCurrent()
{
    return &tasks[currentTID];
}