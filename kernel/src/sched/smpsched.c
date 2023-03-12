#include <sched/smpsched.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <cpu/lapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <drv/serial.h>

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

void taskA()
{
    while (1)
    {
        uint8_t val;
        iasm("inb %%dx,%%al"
             : "=a"(val)
             : "d"(COM1 + 5));

        while (!(val & 0b100000)) // wait for input buffer to be clear
            ;

        iasm("outb %0, %1" ::"a"((uint8_t)'A'), "Nd"(COM1));
    }
}

void taskB()
{
    while (1)
    {
        uint8_t val;
        iasm("inb %%dx,%%al"
             : "=a"(val)
             : "d"(COM1 + 5));

        while (!(val & 0b100000)) // wait for input buffer to be clear
            ;

        iasm("outb %0, %1" ::"a"((uint8_t)'B'), "Nd"(COM1));
    }
}

void taskC()
{
    while (1)
    {
        uint8_t val;
        iasm("inb %%dx,%%al"
             : "=a"(val)
             : "d"(COM1 + 5));

        while (!(val & 0b100000)) // wait for input buffer to be clear
            ;

        iasm("outb %0, %1" ::"a"((uint8_t)'C'), "Nd"(COM1));
    }
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

void schedAdd(void *entry, bool kernel)
{
    sched_task_t *t = schedLast(0);           // get last task of the next core
    t->next = blkBlock(sizeof(sched_task_t)); // allocate next
    TASK(t->next)->prev = t;                  // set previous task
    t = TASK(t->next);                        // point to the newly allocated task
    zero(t, sizeof(sched_task_t));            // clear it

    // metadata
    lock(schedLock, { t->id = lastTaskID++; }); // set ID

    // essential registers
    t->registers.rflags = 0b11001000000010;                                                       // enable interrupts and iopl
    t->registers.rsp = t->registers.rbp = (uint64_t)pmmPages(K_STACK_SIZE / 4096) + K_STACK_SIZE; // create a new stack
    t->registers.rip = (uint64_t)entry;                                                           // set instruction pointer

    // segment registers
    if (kernel)
    {
        t->registers.cs = 8 * 1;
        t->registers.ss = 8 * 2;
    }
    else
    {
        t->registers.cs = (8 * 4) | 3;
        t->registers.ss = (8 * 3) | 3;
    }

    // page table
    vmm_page_table_t *pt = vmmCreateTable(false, false);
    t->registers.cr3 = (uint64_t)pt;

    for (int i = 0; i < K_STACK_SIZE; i += 4096) // map stack
        vmmMap(pt, (void *)t->registers.rsp - i, (void *)t->registers.rsp - i, true, true, false, false);

    vmmMap(pt, (void *)t->registers.rip, (void *)t->registers.rip, true, true, false, false); // map executable
}

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

#if 0
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
#endif
    }

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

void schedInit()
{
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

    // temporary, append a task to each core
    void *taskAPage = pmmPage();
    memcpy8(taskAPage, taskA, 4096);

    void *taskBPage = pmmPage();
    memcpy8(taskBPage, taskB, 4096);

    void *taskCPage = pmmPage();
    memcpy8(taskCPage, taskC, 4096);

    int q = 0;
    for (int i = 0; i < maxCore; i++)
    {
        switch (q++)
        {
        case 0:
            schedAdd(taskAPage, false);
            break;
        case 1:
            schedAdd(taskBPage, false);
            break;
        case 2:
            schedAdd(taskCPage, false);
            break;
        default:
            schedAdd(taskAPage, false);
            q = 0;
            break;
        }
    }
}

void schedEnable()
{
    _enabled = true;
    sti();
    commonTask();
    while (1)
        ;
}