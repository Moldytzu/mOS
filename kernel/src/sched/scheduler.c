#include <sched/scheduler.h>
#include <sched/hpet.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <cpu/xapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <drv/serial.h>
#include <subsys/vt.h>
#include <main/panic.h>
#include <stdnoreturn.h>
#include <fw/acpi.h>

// #define BENCHMARK

#define TASK(x) ((sched_task_t *)x)

bool taskKilled[K_MAX_CORES];         // indicates that last task was killed
sched_task_t queueStart[K_MAX_CORES]; // start of the linked lists
sched_task_t *lastTask[K_MAX_CORES];  // current task in the linked list
uint16_t lastCore = 0;                // core on which last task was added
uint32_t lastTaskID = 0;              // last id of the last task addedd
uint16_t maxCore = 0;

locker_t schedLock;

bool _enabled = false;

void callWithStack(void *func, void *stack);

void commonTask()
{
    while (1)
    {
        sti();
        iasm("int $0x20");
    }
}

// determine length of tasks
uint16_t queueLen(uint16_t core)
{
    uint16_t len = 0;
    sched_task_t *t = &queueStart[core];

    while (t->next)
    {
        len++;
        t = (sched_task_t *)t->next;
    }

    return len;
}

// determine to which core we should add the the task
ifunc uint16_t nextCore()
{
    // introducing LoadBalancing™:
    // find least used core and add task to it

    uint16_t leastUsedCore = 0;
    uint16_t leastUsedLen = 1000;
    for (int i = 0; i < maxCore; i++)
    {
        if (queueLen(i) < leastUsedLen)
        {
            leastUsedLen = queueLen(i);
            leastUsedCore = i;
        }
    }

    return leastUsedCore;
}

// first task of a core
ifunc sched_task_t *schedFirst(uint16_t core)
{
    return &queueStart[core];
}

// last task of a core
sched_task_t *schedLast(uint16_t core)
{
    sched_task_t *t = schedFirst(core);

    while (TASK(t->next))
        t = TASK(t->next);

    return t;
}

// add new task
sched_task_t *schedAdd(const char *name, void *entry, uint64_t stackSize, void *execBase, uint64_t execSize, uint64_t terminal, const char *cwd, int argc, char **argv, bool elf, bool driver)
{
    uint32_t id = nextCore(); // get next core id

    sched_task_t *t = blkBlock(sizeof(sched_task_t));

    // metadata
    t->id = lastTaskID++;                          // set ID
    t->core = id;                                  // set core id
    memcpy(t->name, name, min(strlen(name), 128)); // set a name
    t->lastVirtualAddress = TASK_BASE_ALLOC;
    t->terminal = terminal;
    t->isElf = elf;
    t->isDriver = driver;

    // registers
    void *stack = t->stackBase = pmmPages(K_STACK_SIZE / VMM_PAGE + 1 /*one page for arguments*/);
    if (driver)
        t->registers.rflags = 0b11001000000010; // enable interrupts and set IOPL to 3
    else
        t->registers.rflags = 0b1000000010;                                          // enable interrupts
    t->registers.rsp = t->registers.rbp = (uint64_t)stack + K_STACK_SIZE - VMM_PAGE; // set the new stack
    t->registers.rip = TASK_BASE_ADDRESS + (uint64_t)entry;                          // set instruction pointer

    // segment registers
    t->registers.cs = (8 * 4) | 3;
    t->registers.ss = (8 * 3) | 3;

    // page table
    vmm_page_table_t *pt = vmmCreateTable(false);
    t->registers.cr3 = (uint64_t)pt;
    t->pageTable = pt;

    for (int i = 0; i < K_STACK_SIZE + VMM_PAGE; i += VMM_PAGE) // map stack
        vmmMap(pt, (void *)t->registers.rsp - i, (void *)t->registers.rsp - i, VMM_ENTRY_RW | VMM_ENTRY_USER);

    for (size_t i = 0; i < execSize; i += VMM_PAGE) // map task as user, read-write
        vmmMap(pt, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, VMM_ENTRY_RW | VMM_ENTRY_USER);

    t->elfBase = execBase;
    t->elfSize = execSize;

    // handle c arguments
    argc = min(argc, K_SCHED_MAX_ARGUMENTS);

    t->registers.rdi = 1 + argc; // arguments count the path + the optional arguments
    t->registers.rsi = t->registers.rsp;

    void **arguments = (void **)t->registers.rsi;                                        // buffer in which we will put our arguments
    char *str = (char *)(t->registers.rsi + (K_SCHED_MAX_ARGUMENTS * sizeof(uint64_t))); // buffer with which we will copy the strings

    memcpy(str, name, strlen(name)); // copy name
    arguments[0] = str;              // point to it
    str += strlen(name);             // move pointer after it
    *str++ = '\0';                   // terminate string

    for (int i = 0; i < argc; i++) // put every argument
    {
        size_t len = strlen(argv[i]);

        memcpy(str, argv[i], len); // copy argument
        arguments[i + 1] = str;    // point to it

        if ((uint64_t)str + len >= t->registers.rsp + VMM_PAGE) // don't overflow
            break;

        str += len;    // move after text
        *str++ = '\0'; // terminate string
    }

    // memory fields
    t->allocated = pmmPage();                // the array to store the allocated addresses (holds 1 page address until an allocation occurs)
    t->allocatedIndex = 0;                   // the current index in the array
    t->allocatedBufferPages++;               // we have one page already allocated
    t->lastVirtualAddress = TASK_BASE_ALLOC; // set the last address

    // enviroment
    t->enviroment = pmmPage(); // 4k should be enough for now
    if (!cwd)
        t->cwd[0] = '/'; // set the current working directory to the root
    else
        memcpy(t->cwd, cwd, min(strlen(cwd), 512)); // copy the current working directory

#ifdef K_SCHED_DEBUG
    logDbg(LOG_SERIAL_ONLY, "sched: adding task %s", t->name);
#endif

    lock(schedLock, {
        sched_task_t *last = schedLast(id); // get last task

        t->prev = last; // set previous
        last->next = t; // add our new task in list
    });

    return t;
}

uint8_t simdContext[512][K_MAX_CORES];

// do the context switch
void schedSchedule(idt_intrerrupt_stack_t *stack)
{
    if (!_enabled)
        return;

    uint64_t id = smpID();

    iasm("fxsave %0 " ::"m"(simdContext[id])); // save simd context

    lock(schedLock, {
        if (!taskKilled[id])
        {
            if (queueStart[id].next == lastTask[id] && !lastTask[id]->next && id != 0) // if we only have one thread running on the core don't reschedule
            {
                release(schedLock);
                return;
            }

            if (lastTask[id]->quantumLeft) // wait for the quantum to be reached
            {
                lastTask[id]->quantumLeft--;
                release(schedLock);
                return;
            }

            // we've hit the commonTask
            if (lastTask[id] == &queueStart[id])
            {
                if (id == 0) // update screen
                {
                    switch (vtGetMode())
                    {
                    case VT_DISPLAY_FB:
                        // todo: copy the user display framebuffer to the global framebuffer
                        break;
                    case VT_DISPLAY_TTY0:
#ifdef BENCHMARK
                        uint64_t a = hpetMillis();
#endif
                        framebufferZero();
#ifdef BENCHMARK
                        uint64_t b = hpetMillis();
#endif
                        framebufferWrite(vtGet(0)->buffer);
#ifdef BENCHMARK
                        uint64_t c = hpetMillis();
#endif
                        framebufferWritec(K_FB_CURSOR);
#ifdef K_FB_DOUBLE_BUFFER
                        framebufferUpdate();
#endif
#ifdef BENCHMARK
                        uint64_t d = hpetMillis();
                        logInfo("zero took %d, writing buffer took %d, updating took %d", b - a, c - b, d - c);
#endif
                        break;
                    case VT_DISPLAY_KERNEL:
                    default: // doesn't update the framebuffer and lets the kernel write things to it
                        break;
                    }
                }
            }

            // set new quantum
            lastTask[id]->quantumLeft = K_SCHED_MIN_QUANTUM;

#ifdef K_SCHED_DEBUG
            logDbg(LOG_SERIAL_ONLY, "sched: saving task %s", lastTask[id]->name);
#endif

            // save old state
            memcpy(&lastTask[id]->registers, stack, sizeof(idt_intrerrupt_stack_t));

            // save old simd context
            memcpy(&lastTask[id]->simdContext, simdContext, 512);
        }
        else
            taskKilled[id] = false; // reset the flag if needed

        // get next id
        lastTask[id] = lastTask[id]->next;
        if (!lastTask[id])
        {
            if (queueStart[id].next && id != 0) // start queue after the common task to improve efficiency (note: first core has to hit the common task to do house keeping tasks like updating the framebuffer)
                lastTask[id] = (sched_task_t *)queueStart[id].next;
            else
                lastTask[id] = &queueStart[id];
        }

#ifdef K_SCHED_DEBUG
        logDbg(LOG_SERIAL_ONLY, "sched: loading task %s", lastTask[id]->name);
#endif

        // copy new state
        memcpy(stack, &lastTask[id]->registers, sizeof(idt_intrerrupt_stack_t));

        // copy new simd context
        memcpy(simdContext, &lastTask[id]->simdContext, 512);

        iasm("fxrstor %0 " ::"m"(simdContext)); // restore simd context
    });
}

// initialise the scheduler
void schedInit()
{
    vtCreate(); // create the very first terminal (the full screen one)

    maxCore = smpCores();
    zero(queueStart, sizeof(queueStart));
    zero(taskKilled, sizeof(taskKilled));

    lastTaskID = 1;

    for (int i = 0; i < maxCore; i++) // set start of the queues to the common task
    {
        sched_task_t *t = &queueStart[i];
        memcpy(t->name, "common task", strlen("common task"));
        t->registers.rflags = 0b1000000010; // interrupts
        t->registers.cs = 8;
        t->registers.ss = 16;
        t->registers.rsp = t->registers.rbp = (uint64_t)pmmPage() + PMM_PAGE;
        t->registers.rip = (uint64_t)commonTask;
        t->registers.cr3 = (uint64_t)vmmGetBaseTable();

        lastTask[i] = t;
    }
}

// get current task
sched_task_t *schedGetCurrent(uint32_t core)
{
    return lastTask[core];
}

// get the task with id
sched_task_t *schedGet(uint32_t id)
{
    for (int i = 0; i < smpCores(); i++)
    {
        sched_task_t *t = schedFirst(i);
        while (t)
        {
            if (t->id == id)
                return t;
            t = t->next;
        }
    }

    return NULL;
}

// kill a task
void schedKill(uint32_t id)
{
    if (id == 1)
        panick("Attempt to kill the init system.");

    sched_task_t *task;

    // remove task from list
    lock(schedLock, {
        task = schedGet(id); // get task structure from id

        // release resources
        for (int i = 0; i < task->allocatedIndex; i++)
            if (task->allocated[i] != NULL)
                pmmDeallocate(task->allocated[i]);

        pmmDeallocatePages(task->allocated, task->allocatedBufferPages);
        pmmDeallocatePages(task->elfBase, task->elfSize / VMM_PAGE);
        pmmDeallocatePages(task->stackBase, K_STACK_SIZE / VMM_PAGE + 1);
        pmmDeallocate(task->enviroment);
        vmmDestroy(task->pageTable);
        blkDeallocate(task);

        TASK(task->prev)->next = task->next; // remove task from its list
        taskKilled[smpID()] = true;          // signal that we have killed a task (todo: make it so we know which task was killed so we can kill other tasks beside the current running one)
    });
}

// enable the scheduler for the current core
void schedEnable()
{
    if (smpID())
        logInfo("Jumping in userspace");

    _enabled = true;
    callWithStack(commonTask, (void *)lastTask[smpID()]->registers.rsp);
    while (1)
        ;
}