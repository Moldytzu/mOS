#include <sched/scheduler.h>
#include <sched/pit.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <cpu/gdt.h>
#include <fw/bootloader.h>
#include <drv/serial.h>
#include <main/panic.h>
#include <sys/syscall.h>

void *kernelStack;
struct vt_terminal *firstTerminal; // first usable terminal
struct sched_task rootTask;        // root of the tasks list
struct sched_task *currentTask;    // current task in the tasks list
uint32_t lastTID = 0;              // last task ID
bool enabled = false;              // enabled
bool taskKilled = false;           // flag that indicates if task was killed
bool skipSaving = false;           // flag that indicates if we should skip saving registers next tick

extern void userspaceJump(uint64_t rip, uint64_t stack);

// idle task
void idleTask()
{
    while (1)
        ;
}

uint8_t simdContext[512] __attribute__((aligned(16)));

// schedule the next task
void optimize schedulerSchedule(struct idt_intrerrupt_stack *stack)
{
    if (!enabled)
        return; // don't do anything if it isn't enabled

    vmmSwap(vmmGetBaseTable()); // swap the page table

    iasm("fxsave %0 " ::"m"(simdContext)); // save simd context

    if (currentTask->sleep)
        pitSet(200); // reset the timings

    // handle vt mode and calculate cpu time only after switching the idle task
    if (currentTask->id != 0)
        goto c;

    switch (vtGetMode())
    {
    case VT_DISPLAY_KERNEL:
        // do nothing
        break;
    case VT_DISPLAY_FB:
        // todo: copy the user display framebuffer to the global framebuffer
        break;
    case VT_DISPLAY_TTY0:
        framebufferClear(0);
        framebufferWrite(vtGet(0)->buffer);
        break;
    default:
        break;
    }

    register uint32_t syscalls = syscallGetCount(); // get the overall syscall usage

    // calculate the percents for each task
    struct sched_task *task = rootTask.next; // second task
    while (task)
    {
        task->overallCPUpercent = (task->syscallUsage * 100) / syscalls; // multiply everything by 100 so we don't use expensive floating point math
        task->syscallUsage = 1;                                          // reset the counter at one so we don't divide by zero, will be incremented when the task uses any syscall
#ifdef K_SCHED_DEBUG
        printks("sched: %s used %d percent of the total CPU time\n\r", task->name, task->overallCPUpercent);
#endif
        task = task->next;
    }

c:
    if (taskKilled)
    {
        taskKilled = false;
        currentTask = &rootTask; // jump back to the first task
        goto loadnext;           // bypass the saving algorithm and directly load the next task
    }

    if (currentTask->priorityCounter--) // check if the priority counter is over
    {
#ifdef K_SCHED_DEBUG
        printks("sched: %s still has %d ticks left. doing nothing\n\r", currentTask->name, currentTask->priorityCounter + 1);
#endif
        vmmSwap(currentTask->pageTable); // swap the page table
        return;
    }
    currentTask->priorityCounter = currentTask->priority; // reset counter

    if (skipSaving)
    {
        skipSaving = false;
        goto loadnext;
    }

#ifdef K_SCHED_DEBUG
    printks("sched: saving %s\n\r", currentTask->name);
#endif

    // save the registers
    memcpy64(&currentTask->intrerruptStack, stack, sizeof(struct idt_intrerrupt_stack) / sizeof(uint64_t));

    // copy the simd context
    memcpy64(currentTask->simdContext, simdContext, sizeof(currentTask->simdContext) / sizeof(uint64_t));

loadnext:
    // load the next task
    do
    {
        if (currentTask->next == NULL) // wrap around
            currentTask = &rootTask;
        else
            currentTask = currentTask->next;

        if (currentTask->sleep) // if the task is in sleep
        {
#ifdef K_SCHED_DEBUG
            printks("sched: %s is sleeping for %d ticks\n\r", currentTask->name, currentTask->sleep);
#endif
            currentTask->sleep--; // decrement the counter
            goto loadnext;        // skip
        }

    } while (currentTask->state != 0);

#ifdef K_SCHED_DEBUG
    printks("sched: loading %s\n\r", currentTask->name);
#endif

    // copy the new registers
    memcpy64(stack, &currentTask->intrerruptStack, sizeof(struct idt_intrerrupt_stack) / sizeof(uint64_t));

    // copy the new simd context
    memcpy64(simdContext, currentTask->simdContext, sizeof(currentTask->simdContext) / sizeof(uint64_t));

    iasm("fxrstor %0 " ::"m"(simdContext)); // restore simd context

    vmmSwap((void *)currentTask->intrerruptStack.cr3); // swap the page table
}

// initialize the scheduler
void schedulerInit()
{
    kernelStack = mmAllocatePage();                                       // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack + VMM_PAGE;                  // set kernel stack in tss
    memset64(&rootTask, 0, sizeof(struct sched_task) / sizeof(uint64_t)); // clear the root task
    currentTask = &rootTask;                                              // set the current task

    firstTerminal = vtCreate(); // create the first terminal

    void *task = mmAllocatePage();                                         // create an empty page just for the idle task
    memcpy8(task, (void *)idleTask, VMM_PAGE);                             // copy the executable part
    schedulerAdd("Idle Task", 0, VMM_PAGE, task, VMM_PAGE, 0, 0, 0, 0, 0); // create the idle task
}

// enable the scheduler and then jump in the first task
void schedulerEnable()
{
    cli();                                                          // disable intrerrupts, those will be enabled using the rflags
    enabled = true;                                                 // enable the scheduler
    vmmSwap(rootTask.pageTable);                                    // swap the page table
    userspaceJump(TASK_BASE_ADDRESS, rootTask.intrerruptStack.rsp); // jump in userspace
}

// add new task in the queue
struct sched_task *schedulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf)
{
    struct sched_task *task = &rootTask; // first task

    if (task->pageTable) // check if the root task is valid
    {
        while (task->next) // get last task
            task = task->next;

        if (task->pageTable)
        {
            task->next = mmAllocatePage();                                         // allocate next task if the current task is valid
            memset64(task->next, 0, sizeof(struct sched_task) / sizeof(uint64_t)); // clear the thread
            task->next->previous = task;                                           // set the previous task
            task = task->next;                                                     // set current task to the newly allocated task
        }
    }

    uint16_t index = lastTID++;

    // metadata
    task->priorityCounter = 0;                       // reset counter
    task->id = index;                                // set the task ID
    task->priority = 0;                              // switch imediately
    task->terminal = terminal;                       // terminal
    task->elf = elf;                                 // elf status
    task->elfBase = execBase;                        // base of elf
    task->elfSize = execSize;                        // size of elf
    memcpy8(task->name, (char *)name, strlen(name)); // set the name

    // page table
    struct vmm_page_table *newTable = vmmCreateTable(false); // create a new page table
    task->pageTable = newTable;                              // set the new page table

    void *stack = mmAllocatePages(stackSize / VMM_PAGE); // allocate stack for the task
    memset64(stack, 0, stackSize / sizeof(uint64_t));    // clear the stack

    for (size_t i = 0; i < stackSize; i += VMM_PAGE) // map task stack as user, read-write
        vmmMap(newTable, (void *)stack + i, stack + i, true, true);

    for (size_t i = 0; i < execSize; i += VMM_PAGE)
        vmmMap(newTable, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, true, true); // map task as user, read-write

    // initial registers
    task->intrerruptStack.rip = TASK_BASE_ADDRESS + (uint64_t)entry; // set the entry point a.k.a the instruction pointer
    task->intrerruptStack.rflags = 0x202;                            // rflags, enable intrerrupts
    task->intrerruptStack.rsp = (uint64_t)stack + stackSize;         // task stack pointer
    task->intrerruptStack.rbp = task->intrerruptStack.rsp;           // stack frame pointer
    task->intrerruptStack.cs = 0x23;                                 // code segment for user
    task->intrerruptStack.ss = 0x1B;                                 // data segment for user
    task->intrerruptStack.cr3 = (uint64_t)task->pageTable;           // page table

    // arguments
    task->intrerruptStack.rdi = 1 + argc;        // arguments count (1, the name)
    task->intrerruptStack.rsi = (uint64_t)stack; // the stack contains the array

    uint64_t offset = sizeof(void *) * (1 + argc) + 1; // count of address

    memcpy(stack + offset, name, strlen(name));        // copy the name
    *((uint64_t *)stack) = (uint64_t)(stack + offset); // point to the name
    offset += strlen(name) + 1;                        // move the offset after the name

    for (int i = 0; i < argc; i++)
    {
        memcpy(stack + offset, argv[i], strlen(argv[i]));                                   // copy next argument
        *((uint64_t *)(stack + (i + 1) * sizeof(uint64_t *))) = (uint64_t)(stack + offset); // point to the name
        offset += strlen(argv[i]) + 1;                                                      // move the offset after the argument
    }

    // memory fields
    task->allocated = malloc(sizeof(uint64_t));                        // the array to store the allocated addresses (holds 1 page address until an allocation occurs)
    memset64(task->allocated, 0, sizeof(uint64_t) / sizeof(uint64_t)); // null its content
    task->allocatedIndex = 0;                                          // the current index in the array
    task->lastVirtualAddress = (void *)TASK_BASE_ALLOC;                // set the last address

    // enviroment
    task->enviroment = mmAllocatePage();                        // 4k should be enough for now
    memset64(task->enviroment, 0, VMM_PAGE / sizeof(uint64_t)); // clear the enviroment
    if (!cwd)
        task->cwd[0] = '/'; // set the current working directory to the root
    else
        memcpy(task->cwd, cwd, strlen(cwd)); // copy the current working directory
    task->syscallUsage = 1;

#ifdef K_SCHED_DEBUG
    printks("sched: added %s\n\r", name);
#endif

    return task;
}

// get current task
struct sched_task *schedulerGetCurrent()
{
    return currentTask;
}

// return enabled status
bool schedulerEnabled()
{
    return enabled;
}

// set priority to a task
void schedulerPrioritize(uint32_t tid, uint8_t priority)
{
    if (lastTID - 1 < tid) // out of bounds
        return;

    if (priority == 0) // minimum ticks until switching is 1
        priority = 1;

    struct sched_task *task = schedulerGet(tid);
    task->priority = priority;              // set new priority level
    task->priorityCounter = task->priority; // reset counter
}

// set terminal to a task
void schedulerSetTerminal(uint32_t tid, uint32_t terminal)
{
    if (lastTID - 1 < tid) // out of bounds
        return;

    struct sched_task *task = schedulerGet(tid);

    if (!task)
        return; // didn't find it

    task->terminal = terminal; // set new terminal
}

// get the task with the id
struct sched_task *schedulerGet(uint32_t tid)
{
    struct sched_task *task = &rootTask; // first task
    while (task)
    {
        if (task->id == tid)
            break;
        task = task->next;
    }

    return task; // return the task
}

// kill the task with the id
void schedulerKill(uint32_t tid)
{
    if (tid == 1)
        panick("Attempt to kill the init system.");

    struct sched_task *task = schedulerGet(tid);

    if (!task)
        return;

    // deallocate the terminal if it's not used by another task
    uint64_t found = 0;
    struct sched_task *temp = &rootTask; // first task
    while (temp->next)                   // iterate thru every task
    {
        if (temp->terminal == task->terminal)
            found++;
        temp = temp->next;
    }

    if (found == 1) // if only one task is using that task (the task we're killing) then we deallocate the terminal
        vtDestroy(vtGet(task->terminal));

    // deallocate the memory allocations
    for (int i = 0; i < task->allocatedIndex; i++)
        if (task->allocated[i] != NULL)
            mmDeallocatePage(task->allocated[i]);

    free(task->allocated);

    // deallocate the elf (if present)
    if (task->elf)
        mmDeallocatePages(task->elfBase, task->elfSize / VMM_PAGE + 1);

    // deallocate the task
    struct sched_task *prev = task->previous;
    prev->next = task->next;     // bypass this node
    vmmDestroy(task->pageTable); // destroy the page table
    mmDeallocatePage(task);      // free the task

    taskKilled = true;

    // halt until next intrerrupt fires
    sti();
    hlt();

    while (1)
        ; // prevent returning back
}

// get last id
uint32_t schedulerGetLastID()
{
    return lastTID;
}

// skip saving registers next cycle
void schedulerSkipNextSaving()
{
    skipSaving = true;
}