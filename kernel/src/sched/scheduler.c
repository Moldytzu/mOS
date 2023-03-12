#include <sched/scheduler.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <cpu/lapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <drv/serial.h>
#include <subsys/vt.h>

#define TASK(x) ((sched_task_t *)x)

sched_task_t queueStart[K_MAX_CORES]; // start of the linked lists
sched_task_t *lastTask[K_MAX_CORES];  // current task in the linked list
uint64_t minQuantum[K_MAX_CORES];     // minimum quantum of a task
uint16_t lastCore = 0;                // core on which last task was added
uint32_t lastTaskID = 0;              // last id of the last task addedd
uint16_t maxCore = 0;

locker_t schedLock;

bool _enabled = false;

void commonTask()
{
    while (1)
        iasm("int $0x20"); // yield directly
}

// determine to which core we should add the the task
ifunc uint16_t nextCore()
{
    lock(schedLock, {
        lastCore++;

        if (lastCore == maxCore)
            lastCore = 0;
    });

    return lastCore;
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

    sched_task_t *t = schedLast(id);          // get last task of the next core
    t->next = blkBlock(sizeof(sched_task_t)); // allocate next
    TASK(t->next)->prev = t;                  // set previous task
    t = TASK(t->next);                        // point to the newly allocated task
    zero(t, sizeof(sched_task_t));            // clear it

    // metadata
    lock(schedLock, { t->id = lastTaskID++; }); // set ID
    t->core = id;                               // set core id
    memcpy(t->name, name, strlen(name));        // set a name
    t->lastVirtualAddress = TASK_BASE_ALLOC;
    t->terminal = terminal;
    t->isElf = elf;

    // essential registers
    void *stack = pmmPages(K_STACK_SIZE / 4096);
    t->registers.rflags = 0b1000000010;                                   // enable interrupts
    t->registers.rsp = t->registers.rbp = (uint64_t)stack + K_STACK_SIZE; // set the new stack
    t->registers.rip = TASK_BASE_ADDRESS + (uint64_t)entry;               // set instruction pointer

    // segment registers
    t->registers.cs = (8 * 4) | 3;
    t->registers.ss = (8 * 3) | 3;

    // page table
    vmm_page_table_t *pt = vmmCreateTable(false, false);
    t->registers.cr3 = (uint64_t)pt;
    t->pageTable = (uint64_t)pt;

    for (int i = 0; i < K_STACK_SIZE; i += 4096) // map stack
        vmmMap(pt, (void *)t->registers.rsp - i, (void *)t->registers.rsp - i, true, true, false, false);

    for (size_t i = 0; i < execSize; i += VMM_PAGE)
        vmmMap(pt, (void *)TASK_BASE_ADDRESS + i, (void *)execBase + i, true, true, false, false); // map task as user, read-write

    // arguments (todo: refactor this code to be more readable)
    if (argv)
    {
        t->registers.rdi = 1 + argc;                   // arguments count (1, the name)
        t->registers.rsi = (uint64_t)t->registers.rsp; // the stack contains the array

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
    t->allocated = pmmPage();                // the array to store the allocated addresses (holds 1 page address until an allocation occurs)
    zero(t->allocated, sizeof(uint64_t));    // null its content
    t->allocatedIndex = 0;                   // the current index in the array
    t->allocatedBufferPages++;               // we have one page already allocated
    t->lastVirtualAddress = TASK_BASE_ALLOC; // set the last address

    // enviroment
    t->enviroment = pmmPage();     // 4k should be enough for now
    zero(t->enviroment, VMM_PAGE); // clear the enviroment
    if (!cwd)
        t->cwd[0] = '/'; // set the current working directory to the root
    else
        memcpy(t->cwd, cwd, strlen(cwd)); // copy the current working directory

    return t;
}

// do the context switch
void schedSchedule(idt_intrerrupt_stack_t *stack)
{
    if (!_enabled)
        return;

    uint64_t id = smpID();

    if (lastTask[id]->quantumLeft) // wait for the quantum to be reached
    {
        lastTask[id]->quantumLeft--;
        return;
    }

    // we've hit the commonTask
    if (lastTask[id] == &queueStart[id])
    {
        // adjust quantums based on the lapic frequency (it isn't the same on every machine thus we have to adjust)
        uint64_t *tps = lapicGetTPS();
        minQuantum[id] = (tps[id] / K_SCHED_FREQ) + 1;

        if (id == 0)
        {
            switch (vtGetMode())
            {
            case VT_DISPLAY_FB:
                // todo: copy the user display framebuffer to the global framebuffer
                break;
            case VT_DISPLAY_TTY0:
                framebufferClear(0);
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
    lastTask[id]->quantumLeft = minQuantum[id];

    // save old state
    memcpy(&lastTask[id]->registers, stack, sizeof(idt_intrerrupt_stack_t));

    // get next id
    lastTask[id] = lastTask[id]->next;
    if (!lastTask[id])
        lastTask[id] = &queueStart[id];

    // copy new state
    memcpy(stack, &lastTask[id]->registers, sizeof(idt_intrerrupt_stack_t));

    vmmSwap((void *)lastTask[id]->registers.cr3);
}

// initialise the scheduler
void schedInit()
{
    vtCreate(); // create the very first terminal (the full screen one)

    maxCore = bootloaderGetSMP()->cpu_count;
    zero(queueStart, sizeof(queueStart));

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
        minQuantum[i] = 1;
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
        do
        {
            if (t->id == id)
                return t;

            t = t->next;
        } while (t->next);
    }
}

// kill a task
void schedKill(uint32_t id)
{
    // todo: implement this by back-porting from the old scheduler
}

// enable the scheduler for the current core
void schedEnable()
{
    _enabled = true;
    sti();
    commonTask();
    while (1)
        ;
}