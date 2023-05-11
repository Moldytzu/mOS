#include <sys/syscall.h>
#include <sys/sys.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close, socket, power, driver};
const char *syscallNames[] = {"exit", "write", "read", "input", "display", "exec", "pid", "mem", "vfs", "open", "close", "socket", "power", "driver"};

// open file at relative path
uint64_t openRelativePath(const char *path, sched_task_t *task)
{
    uint64_t fd = 0;
    if (fd = vfsOpen(path)) // check if path exists as is
        return fd;

    // todo: implement more advanced path traversal like .. and .
    if (strlen(path) > 2 && memcmp(path, "./", 2) == 0) // remove ./
        path += 2;

    char *buffer = (char *)pmmPage(); // allocate an internal buffer (todo: this is not cleaned up properly)
    zero(buffer, PMM_PAGE);

    uint64_t offset = strlen(task->cwd);
    memcpy((void *)buffer, task->cwd, offset);             // copy cwd
    memcpy((void *)(buffer + offset), path, strlen(path)); // append given path

    if (fd = vfsOpen(buffer)) // check if it exists
        return fd;

    // fail
    pmmDeallocate(buffer); // clean up

    return 0;
}

// forces a scheduler context swap
void yield()
{
    iasm("int $0x20"); // simulates an interrupt
}

extern void sysretInit();

// handler called on syscall
void syscallHandler(idt_intrerrupt_stack_t *registers)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

#ifdef K_SYSCALL_DEBUG
    printks("syscall: requested %s (0x%x), argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x\n\r", syscallNames[registers->rdi], registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
#endif

    sched_task_t *t = schedGetCurrent(smpID());

    if (registers->rdi < (sizeof(syscallHandlers) / sizeof(void *))) // check if the syscall is in range
    {
        // logDbg(LOG_SERIAL_ONLY, "%s calls %s with arguments %x %x %x %x", t->name, syscallNames[registers->rdi], registers->rsi, registers->rdx, registers->r8, registers->r9);
        syscallHandlers[registers->rdi](registers->rsi, registers->rdx, registers->r8, registers->r9, t); // call the handler
    }

    vmmSwap((void *)registers->cr3); // swap the page table back
}

// init syscall handling
void syscallInit()
{
    sysretInit(); // enable sysret/syscall capability
    logInfo("syscall: enabled syscall/sysret functionality");
}