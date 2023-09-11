#include <sched/scheduler.h>
#include <sched/hpet.h>
#include <misc/logger.h>
#include <cpu/smp.h>
#include <cpu/xapic.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <drv/serial.h>
#include <vt/vt.h>
#include <main/panic.h>
#include <stdnoreturn.h>
#include <fw/acpi.h>
#include <fs/vfs.h>

// #define BENCHMARK

#define TASK(x) ((sched_task_t *)x)

sched_task_t queueStart[K_MAX_CORES]; // start of the linked lists
sched_task_t *lastTask[K_MAX_CORES];  // current task in the linked list
uint16_t lastCore = 0;                // core on which last task was added
uint32_t lastTaskID = 0;              // last id of the last task addedd
uint16_t maxCore = 0;                 // max available cores

spinlock_t schedLock[K_MAX_CORES];

bool _enabled[K_MAX_CORES];

void callWithStack(void *func, void *stack);

// filler task
void commonTask()
{
    while (1)
        iasm("int $0x20");
}

// handles framebuffer updates
void framebufferTask()
{
    // note: we run this with interrupts disabled to not have context switches in between random pieces of code
    while (1)
    {
        switch (vtGetMode())
        {
        case VT_DISPLAY_FB:
            framebufferUpdate(); // we provide userspace the back buffer thus we just have to update the screen
            break;
        case VT_DISPLAY_TTY0:
        {
#ifdef BENCHMARK
            uint64_t a = hpetMillis();
#endif

            framebufferZero();

            struct limine_framebuffer fb = framebufferGet();
            psf2_header_t *font = framebufferGetFont();

            char *buffer = (char *)vtGet(0)->buffer;
            size_t bufferSize = strlen(buffer);

            if (!bufferSize) // nothing to display
                break;

            // emulate framebuffer's cursor behaviour to determine maximum characters we can fit on the screen
            size_t x = 0;
            size_t y = 0;
            size_t printableCharacters = 0;

            for (size_t i = bufferSize - 1; i; i--) // since we want to print only the last few visible characters we begin to search from the end
            {
                // this is virtualy the same algorithm as in framebufferWritec except the drawing part
                char c = buffer[i];
                if (c == '\n' || x + font->width > fb.width)
                {
                    y += font->height + 1;
                    x = 0;

                    if (y + font->height + 1 >= fb.height) // maximum reached
                        break;
                }

                if (c != '\n')
                    x += font->width;

                printableCharacters++;
            }

            // draw the printable characters
            for (size_t i = bufferSize - printableCharacters; i < bufferSize; i++)
                framebufferWritec(buffer[i]);

            framebufferWritec(K_FB_CURSOR); // draw the cursor

            framebufferUpdate();
#ifdef BENCHMARK
            uint64_t b = hpetMillis();
            logDbg(LOG_SERIAL_ONLY, "updating framebuffer took %d miliseconds (%d characters written)", b - a, printableCharacters);
#endif
        }
        break;
        case VT_DISPLAY_FB_DOUBLE_BUFFERED: // apps choose when to update the screen
        case VT_DISPLAY_KERNEL:
        default: // doesn't update the framebuffer and lets the kernel write things to it
            break;
        }

        iasm("int $0x20"); // reschedule
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

    sched_task_t *t = pmmPage(); // there aren't any benefits of using the block allocator because
                                 // it's a fairly large structure and wouldn't fit nicely in half a page

    // metadata
    t->id = lastTaskID++;                          // set ID
    t->core = id;                                  // set core id
    memcpy(t->name, name, min(strlen(name), 128)); // set a name
    t->lastVirtualAddress = TASK_BASE_ALLOC;
    t->terminal = terminal;
    t->isElf = elf;
    t->isDriver = driver;

    if (driver) // give speed advantage to regular apps
        t->targetQuantum = 0;
    else
        t->targetQuantum = K_SCHED_MIN_QUANTUM;

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

    vmmMap(pt, &t->registers, &t->registers, 0);

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

    // mailCompose(&t->mailbox, 5, 1, "hello mailbox", 13);

    lock(schedLock[id], {
        sched_task_t *last = schedLast(id); // get last task

        t->prev = last; // set previous
        last->next = t; // add our new task in list
    });

    return t;
}

extern void saveSimdContextTo(void *simdContext);

// do the context switch (it's guranteed to be called after initialisation and enable)
void schedSchedule(idt_intrerrupt_stack_t *stack)
{
    uint64_t id = smpID();

    lock(schedLock[id], {
        if (queueStart[id].next == lastTask[id] && !lastTask[id]->next && id != 0) // if we only have one thread running on the application core don't reschedule
        {
            release(schedLock[id]);
            return;
        }

        if (lastTask[id]->quantumLeft) // wait for the quantum to be reached
        {
#ifdef K_SCHED_DEBUG
            logDbg(LOG_SERIAL_ONLY, "sched: %s has %d quantum. switching back", lastTask[id]->name, lastTask[id]->quantumLeft);
#endif

            lastTask[id]->quantumLeft--;
            release(schedLock[id]);
            return;
        }

        lastTask[id]->quantumLeft = lastTask[id]->targetQuantum; // reset quantum

        // save task context
        saveSimdContextTo(&lastTask[id]->simdContext);                                                // save simd context
        memcpy64(&lastTask[id]->registers, stack, sizeof(idt_intrerrupt_stack_t) / sizeof(uint64_t)); // save old registers

#ifdef K_SCHED_DEBUG
        logDbg(LOG_SERIAL_ONLY, "sched: saving task %s", lastTask[id]->name);
#endif
    });

    schedSwitchNext(); // load next context
    unreachable();     // hint we won't return
}

extern void switchTo(void *stack, void *simdContext);

// performs context switch to next context
void schedSwitchNext()
{
    uint64_t id = smpID();

    lock(schedLock[id], {
        // get next task
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
    });

    switchTo(&lastTask[id]->registers, &lastTask[id]->simdContext); // switch to the context
    unreachable();                                                  // hint we won't return
}

// reschedule if scheduler is unlocked
void schedScheduleIfPossible()
{
    pause();

#ifdef K_ATOMIC_RESCHEDULE
    bool canReschedule = _enabled[smpID()] && !ATOMIC_IS_LOCKED(schedLock[smpID()]);
    if (canReschedule)
        iasm("int $0x20");
#endif
}

// initialise the scheduler
void schedInit()
{
    vtCreate(); // create the very first terminal (the full screen one)

    maxCore = smpCores();
    zero(queueStart, sizeof(queueStart));
    zero(_enabled, sizeof(_enabled));

    lastTaskID = 1;

    // initial task to update framebuffer
    sched_task_t *t = &queueStart[0];
    sprintf(t->name, "framebuffer update");
    t->registers.rflags = 0b0000000010; // interrupts disabled
    t->registers.cs = 8;
    t->registers.ss = 16;
    t->registers.rsp = t->registers.rbp = (uint64_t)pmmPage() + PMM_PAGE;
    t->registers.rip = (uint64_t)framebufferTask;
    t->registers.cr3 = (uint64_t)vmmGetBaseTable();
    t->targetQuantum = 0;

    lastTask[0] = t;

    // set start of the application cores' queues to the common task
    for (int i = 1; i < maxCore; i++)
    {
        sched_task_t *t = &queueStart[i];
        sprintf(t->name, "common task %d", i);
        t->registers.rflags = 0b1000000010; // interrupts
        t->registers.cs = 8;
        t->registers.ss = 16;
        t->registers.rsp = t->registers.rbp = (uint64_t)pmmPage() + PMM_PAGE;
        t->registers.rip = (uint64_t)commonTask;
        t->registers.cr3 = (uint64_t)vmmGetBaseTable();
        t->targetQuantum = 0;

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

    sched_task_t *task = schedGet(id); // get task structure from id

    if (!task) // doesn't exist
        return;

    lock(schedLock[task->core], {
        // release resources

        // deallocate allocated memory
        for (int i = 0; i < task->allocatedIndex; i++)
            if (task->allocated[i] != NULL)
                pmmDeallocate(task->allocated[i]);

        pmmDeallocatePages(task->allocated, task->allocatedBufferPages);

        // close all files
        for (int i = 0; i < TASK_MAX_FILE_DESCRIPTORS; i++)
            vfsClose(task->fileDescriptorPointers[i]);

        // deallocate mails
        mailbox_t *mail = mailReadNext(&task->mailbox);
        while (mail)
        {
            mailFree(mail);
            mail = mailReadNext(&task->mailbox);
        }

        // deallocate executable
        pmmDeallocatePages(task->elfBase, task->elfSize / VMM_PAGE);

        // deallocate stack
        pmmDeallocatePages(task->stackBase, K_STACK_SIZE / VMM_PAGE + 1);

        // deallocate enviroment
        pmmDeallocate(task->enviroment);

        // destroy page table
        vmmDestroy(task->pageTable);

        // deallocate task structure
        pmmDeallocate(task);

        TASK(task->prev)->next = task->next; // remove task from its list
    });
}

// enable the scheduler for the current core
void schedEnable()
{
    if (!smpID())
        logInfo("Jumping in userspace");

    _enabled[smpID()] = true;
    callWithStack((void *)queueStart[smpID()].registers.rip, (void *)queueStart[smpID()].registers.rsp); // call core's initial task
    while (1)
        ;
}