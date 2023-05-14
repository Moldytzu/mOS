#include <sched/scheduler.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <cpu/lapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <drv/serial.h>
#include <subsys/vt.h>
#include <main/panic.h>
#include <lai/helpers/pm.h>
#include <lai/helpers/sci.h>
#include <stdnoreturn.h>
#include <fw/acpi.h>

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

// determine to which core we should add the the task
ifunc uint16_t nextCore()
{
    if (lastCore == maxCore)
        lastCore = 0;

    return lastCore++;
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
    void *stack = pmmPages(K_STACK_SIZE / VMM_PAGE + 1 /*one page for arguments*/);
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
    t->pageTable = (uint64_t)pt;

    for (int i = 0; i < K_STACK_SIZE + VMM_PAGE; i += VMM_PAGE) // map stack
        vmmMap(pt, (void *)t->registers.rsp - i, (void *)t->registers.rsp - i, VMM_ENTRY_RW | VMM_ENTRY_USER);

    for (size_t i = 0; i < execSize; i += VMM_PAGE) // map task as user, read-write
        vmmMap(pt, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, VMM_ENTRY_RW | VMM_ENTRY_USER);

    // handle c arguments
    t->registers.rdi = 1 + argc; // arguments count the path + the optional arguments
    t->registers.rsi = t->registers.rsp;

    void **arguments = (void **)t->registers.rsi;                     // buffer in which we will put our 30 max arguments
    char *str = (char *)(t->registers.rsi + (30 * sizeof(uint64_t))); // buffer with which we will copy the strings

    memcpy(str, name, strlen(name)); // copy name
    arguments[0] = str;              // point to it
    str += strlen(name);             // move pointer after it
    *str++ = '\0';                   // terminate string

    for (int i = 0; i < argc; i++) // put every argument
    {
        size_t len = strlen(argv[i]);

        memcpy(str, argv[i], len); // copy argument
        arguments[i + 1] = str;    // point to it

        str += len;    // move after text
        *str++ = '\0'; // terminate string

        if ((uint64_t)str >= t->registers.rsp + VMM_PAGE) // don't overflow
            break;
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

    lock(schedLock, {
        sched_task_t *last = schedLast(id); // get last task

        t->prev = last; // set previous
        last->next = t; // add our new task in list
    });

    return t;
}

// do the context switch
void schedSchedule(idt_intrerrupt_stack_t *stack)
{
    if (!_enabled)
        return;

    uint64_t id = smpID();

    uint8_t simdContext[512];
    iasm("fxsave %0 " ::"m"(simdContext)); // save simd context

    lock(schedLock, {
        if (!taskKilled[id])
        {
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
                        framebufferZero();
                        framebufferWrite(vtGet(0)->buffer);
                        break;
                    case VT_DISPLAY_KERNEL:
                    default: // doesn't update the framebuffer and lets the kernel write things to it
                        break;
                    }
                }
            }

#ifdef K_ACPI_LAI
            // handle sci events like power button
            if (id == 0 && lastTask[id] == &queueStart[id])
            {
                uint16_t event = lai_get_sci_event();
                if (event == ACPI_POWER_BUTTON)
                    acpiShutdown();
            }
#endif

            // set new quantum
            lastTask[id]->quantumLeft = K_SCHED_MIN_QUANTUM;

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
            lastTask[id] = &queueStart[id];

        // copy new state
        memcpy(stack, &lastTask[id]->registers, sizeof(idt_intrerrupt_stack_t));

        // copy new simd context
        memcpy(simdContext, &lastTask[id]->simdContext, 512);

        iasm("fxrstor %0 " ::"m"(simdContext)); // restore simd context

        vmmSwap((void *)lastTask[id]->registers.cr3);
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

    lock(schedLock, {
#ifdef K_SCHED_DEBUG
        uint64_t a = pmmTotal().available;
#endif
        // todo: deallocate resources here!

        sched_task_t *task = schedGet(id);
        TASK(task->prev)->next = task->next; // remove task from its list
        taskKilled[smpID()] = true;          // signal that we have killed a task (todo: make it so we know which task was killed so we can kill other tasks beside the current running one)

#ifdef K_SCHED_DEBUG
        printks("sched: recovered %d KB\n\r", toKB(pmmTotal().available - a));
#endif
    });
}

// enable the scheduler for the current core
void schedEnable()
{
    _enabled = true;
    callWithStack(commonTask, (void *)lastTask[smpID()]->registers.rsp);
    while (1)
        ;
}