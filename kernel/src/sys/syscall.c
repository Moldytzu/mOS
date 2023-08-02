#include <sys/syscall.h>
#include <sys/sys.h>
#include <cpu/idt.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <subsys/vt.h>
#include <misc/logger.h>

void (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close, socket, power, driver, time, perf};
const char *syscallNames[] = {"exit", "write", "read", "input", "display", "exec", "pid", "mem", "vfs", "open", "close", "socket", "power", "driver", "time", "perf"};

// open file at relative path
uint64_t openRelativePath(const char *path, sched_task_t *task)
{
    if (vfsExists(path))      // if a file exists at that given path
        return vfsOpen(path); // open it then return its file descriptor

    // todo: implement more advanced path traversal like .. and .
    if (strlen(path) > 2 && memcmp(path, "./", 2) == 0) // remove ./
        path += 2;

    char *buffer = (char *)pmmPage();         // allocate an internal buffer
    sprintf(buffer, "%s%s", task->cwd, path); // combine cwd and path

    if (vfsExists(buffer)) // check if it exists
    {
        pmmDeallocate(buffer);  // do clean up
        return vfsOpen(buffer); // and return the file descriptor
    }

    // fail
    pmmDeallocate(buffer); // clean up

    return 0;
}

// forces a scheduler context swap
void yield()
{
    iasm("int $0x20"); // simulates an interrupt
}

// handler called on syscall
uint64_t syscallHandler(syscall_stack_t *registers)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

    sched_task_t *task = schedGetCurrent(smpID()); // get task context from scheduler

#ifdef K_SYSCALL_DEBUG
    logDbg(LOG_SERIAL_ONLY, "syscall: %s requested %s (0x%x), argument 1 is 0x%x, argument 2 is 0x%x, return address is 0x%p, argument 3 is 0x%x, argument 4 is 0x%x", t->name, syscallNames[registers->rdi], registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->r8, registers->r9);
#endif

    if (registers->rdi < (sizeof(syscallHandlers) / sizeof(void *)))                                         // check if the syscall is in range
        syscallHandlers[registers->rdi](registers->rsi, registers->rdx, registers->r8, registers->r9, task); // call the handler
    else
        return SYSCALL_STATUS_UNKNOWN_OPERATION;

    return 0;
}

extern void sysretInit();

// init syscall handling
void syscallInit()
{
    sysretInit(); // enable sysret/syscall capability
    logInfo("syscall: enabled syscall/sysret functionality");
}