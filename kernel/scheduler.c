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

// schedule the next task
void schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    if (!enabled)
        return; // don't do anything if it isn't enabled

    vmmSwap(vmmGetBaseTable()); // swap the page table

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

    // copy the new registers
    memcpy(stack, &tasks[currentTID].intrerruptStack, sizeof(struct idt_intrerrupt_stack));

    vmmSwap(tasks[currentTID].pageTable); // swap the page table
}

// initialize the scheduler
void schedulerInit()
{
    memset64(tasks, 0, 0x1000 * sizeof(struct sched_task) / sizeof(uint64_t)); // clear the tasks
    kernelStack = mmAllocatePage();                                            // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack + VMM_PAGE;                       // set kernel stack in tss
}

// enable the scheduler and then jump in the first task
void schedulerEnable()
{
    cli();                                                                   // disable intrerrupts, those will be enabled using the rflags
    enabled = true;                                                          // enable the scheduler
    vmmSwap(tasks[currentTID].pageTable);                                    // swap the page table
    userspaceJump(TASK_BASE_ADDRESS, tasks[currentTID].intrerruptStack.rsp); // jump in userspace
}

// add new task in the queue
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
        vmmMap(newTable, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, true, true); // map task as user, read-write

    // initial registers
    tasks[index].intrerruptStack.rip = TASK_BASE_ADDRESS + (uint64_t)entry; // set the entry point a.k.a the instruction pointer
    tasks[index].intrerruptStack.rflags = 0x202;                            // rflags, enable intrerrupts
    tasks[index].intrerruptStack.rsp = (uint64_t)stack + stackSize;         // task stack
    tasks[index].intrerruptStack.cs = 8 * 4;                                // code segment for user is the first
    tasks[index].intrerruptStack.ss = 8 * 3;                                // data segment for user is the second
}

// get current task
struct sched_task *schedulerGetCurrent()
{
    return &tasks[currentTID];
}