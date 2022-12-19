#include <sched/scheduler.h>
#include <sched/pit.h>
#include <mm/pmm.h>
#include <mm/blk.h>
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
void *toHandle = NULL;
uint32_t toHandleID = 0;

extern void userspaceJump(uint64_t rip, uint64_t stack, uint64_t pagetable);

// idle task
void idleTask()
{
    while (1)
        ;
}

uint8_t simdContext[512] __attribute__((aligned(16)));

extern void callWithPageTable(uint64_t rip, uint64_t pagetable);

// schedule the next task
void schedulerSchedule(idt_intrerrupt_stack_t *stack)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table

    if (!enabled)
        return; // don't do anything if it isn't enabled

    iasm("fxsave %0 " ::"m"(simdContext)); // save simd context

    if (currentTask->sleep)
        pitSet(K_PIT_FREQ); // reset the timings

    // handle vt mode and calculate cpu time only after switching the idle task
    if (currentTask->id != 0)
        goto c;

    if (toHandle)
    {
        struct sched_task *task = schedulerGet(toHandleID);
        callWithPageTable((uint64_t)toHandle, (uint64_t)task->pageTable);
    }

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
    memcpy64(&currentTask->intrerruptStack, stack, sizeof(idt_intrerrupt_stack_t) / sizeof(uint64_t));

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
    memcpy64(stack, &currentTask->intrerruptStack, sizeof(idt_intrerrupt_stack_t) / sizeof(uint64_t));

    // copy the new simd context
    memcpy64(simdContext, currentTask->simdContext, sizeof(currentTask->simdContext) / sizeof(uint64_t));

    iasm("fxrstor %0 " ::"m"(simdContext)); // restore simd context

    vmmSwap((void *)currentTask->intrerruptStack.cr3); // swap the page table
}

// initialize the scheduler
void schedulerInit()
{
    kernelStack = pmmPage();                             // allocate a page for the new kernel stack
    tssGet()->rsp[0] = (uint64_t)kernelStack + VMM_PAGE; // set kernel stack in tss
    zero(&rootTask, sizeof(struct sched_task));          // clear the root task
    currentTask = &rootTask;                             // set the current task

    firstTerminal = vtCreate(); // create the first terminal

    void *task = pmmPage();                                                   // create an empty page just for the idle task
    memcpy8(task, (void *)idleTask, VMM_PAGE);                                // copy the executable part
    schedulerAdd("Idle Task", 0, VMM_PAGE, task, VMM_PAGE, 0, 0, 0, 0, 0, 0); // create the idle task

    printk("sched: initialised\n");
}

// enable the scheduler and then jump in the first task
void schedulerEnable()
{
    cli();                                                                                        // disable intrerrupts, those will be enabled using the rflags
    enabled = true;                                                                               // enable the scheduler
    userspaceJump(TASK_BASE_ADDRESS, rootTask.intrerruptStack.rsp, (uint64_t)rootTask.pageTable); // jump in userspace
}

// add new task in the queue
struct sched_task *schedulerAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver)
{
#ifdef K_SCHED_DEBUG
    uint64_t a = pmmTotal().available;
#endif
    struct sched_task *task = &rootTask; // first task

    if (task->pageTable) // check if the root task is valid
    {
        while (task->next) // get last task
            task = task->next;

        if (task->pageTable)
        {
            task->next = blkBlock(sizeof(struct sched_task)); // allocate next task if the current task is valid
            zero(task->next, sizeof(struct sched_task));      // clear the thread
            task->next->previous = task;                      // set the previous task
            task = task->next;                                // set current task to the newly allocated task
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
    task->isDriver = driver;                         // driver
    memcpy8(task->name, (char *)name, strlen(name)); // set the name

    // page table
    vmm_page_table_t *newTable = vmmCreateTable(driver, driver); // create a new page table
    task->pageTable = newTable;                                  // set the new page table

    void *stack = pmmPages(stackSize / VMM_PAGE); // allocate stack for the task
    zero(stack, stackSize);                       // clear the stack

    // set the data in the structure
    task->stackBase = stack;
    task->stackSize = stackSize;

    for (size_t i = 0; i < stackSize; i += VMM_PAGE) // map task stack as user, read-write
        vmmMap(newTable, (void *)stack + i, stack + i, true, true);

    for (size_t i = 0; i < execSize; i += VMM_PAGE)
        vmmMap(newTable, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, true, true); // map task as user, read-write

    // initial registers
    task->intrerruptStack.rip = TASK_BASE_ADDRESS + (uint64_t)entry; // set the entry point a.k.a the instruction pointer
    task->intrerruptStack.rsp = (uint64_t)stack + stackSize;         // task stack pointer
    task->intrerruptStack.rbp = task->intrerruptStack.rsp;           // stack frame pointer

    task->intrerruptStack.cs = (8 * 4) | 3; // code segment for user
    task->intrerruptStack.ss = (8 * 3) | 3; // data segment for user

    task->intrerruptStack.cr3 = (uint64_t)task->pageTable; // page table

    if (driver)
        task->intrerruptStack.rflags = 0b11001000000010; // enable intrerrupts and iopl
    else
        task->intrerruptStack.rflags = 0b1000000010; // enable intrerrupts

    // arguments
    if (argv)
    {
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
    }

    // memory fields
    task->allocated = blkBlock(sizeof(uint64_t));       // the array to store the allocated addresses (holds 1 page address until an allocation occurs)
    zero(task->allocated, sizeof(uint64_t));            // null its content
    task->allocatedIndex = 0;                           // the current index in the array
    task->lastVirtualAddress = (void *)TASK_BASE_ALLOC; // set the last address

    // enviroment
    task->enviroment = pmmPage();     // 4k should be enough for now
    zero(task->enviroment, VMM_PAGE); // clear the enviroment
    if (!cwd)
        task->cwd[0] = '/'; // set the current working directory to the root
    else
        memcpy(task->cwd, cwd, strlen(cwd)); // copy the current working directory
    task->syscallUsage = 1;

#ifdef K_SCHED_DEBUG
    printks("sched: added %s and wasted %d KB\n\r", name, toKB(a - pmmTotal().available));
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
#ifdef K_SCHED_DEBUG
    uint64_t a = pmmTotal().available;
#endif

    if (tid == 1)
        panick("Attempt to kill the init system.");

    struct sched_task *task = schedulerGet(tid);

    if (!task)
        return;

    // deallocate some fields
    pmmDeallocatePages(task->stackBase, task->stackSize / VMM_PAGE); // stack
    pmmDeallocate(task->enviroment);                                 // enviroment

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
            pmmDeallocate(task->allocated[i]);

    blkDeallocate(task->allocated, task->allocatedIndex * sizeof(uint64_t));

    // deallocate the elf (if present)
    if (task->elf)
        pmmDeallocatePages(task->elfBase, task->elfSize / VMM_PAGE + 1);

    // deallocate the task
    struct sched_task *prev = task->previous;
    prev->next = task->next;                        // bypass this node
    vmmDestroy(task->pageTable);                    // destroy the page table
    blkDeallocate(task, sizeof(struct sched_task)); // free the task

    taskKilled = true;

#ifdef K_SCHED_DEBUG
    printks("sched: recovered %d KB\n\r", toKB(pmmTotal().available - a));
#endif

    // remove idt redirections
    idtClearRedirect(tid);

    // halt until next intrerrupt fires
    sti();
    hlt();

    while (1)
        ; // prevent returning back
}

void schedulerHandleDriver(void *handler, uint32_t tid)
{
    toHandle = handler;
    toHandleID = tid;
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