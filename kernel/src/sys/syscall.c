#include <sys/syscall.h>
#include <sys/sys.h>
#include <cpu/idt.h>
#include <cpu/localStorage.h>
#include <mm/vmm.h>
#include <mm/blk.h>
#include <sched/scheduler.h>
#include <vt/vt.h>
#include <misc/logger.h>

uint64_t (*syscallHandlers[])(uint64_t, uint64_t, uint64_t, uint64_t, sched_task_t *) = {exit, write, read, input, display, exec, pid, mem, vfs, open, close, socket, power, driver, time, perf, mailbox};
const char *syscallNames[] = {"exit", "write", "read", "input", "display", "exec", "pid", "mem", "vfs", "open", "close", "socket", "power", "driver", "time", "perf", "mailbox"};

// push a page on the used array
void pushUsedPage(sched_task_t *task, void *page)
{
    if (task->allocatedIndex + 1 >= ADDRESSES_IN_PAGES(task->allocatedBufferPages)) // check if we can not store the newly allocated page's address
    {
        // calculate required indices
        // NOTE: without the volatile keyword the compiler will f-up the values
        volatile size_t oldPages = task->allocatedBufferPages;
        volatile size_t newPages = ++task->allocatedBufferPages;

        task->allocated = pmmReallocate(task->allocated, oldPages, newPages); // do the reallocation
    }

    task->allocated[task->allocatedIndex++] = page; // store the address to free up later
}

// open file at relative path
uint64_t openRelativePath(const char *path, sched_task_t *task)
{
    // todo: an expandRelativePath function would be great here

    if (vfsExists(path))      // if a file exists at that given path
        return vfsOpen(path); // open it then return its file descriptor

    // todo: implement more advanced path traversal like .. and .
    if (strlen(path) > 2 && memcmp(path, "./", 2) == 0) // remove ./
        path += 2;

    char *buffer = (char *)pmmPage();         // allocate an internal buffer
    sprintf(buffer, "%s%s", task->cwd, path); // combine cwd and path

    if (vfsExists(buffer)) // check if it exists
    {
        uint64_t node = vfsOpen(buffer);

        pmmDeallocate(buffer); // do clean up
        return node;           // return the node
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
uint64_t syscallHandler(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t reserved /*rcx register used by syscall instruction*/, uint64_t r8, uint64_t r9)
{
    vmmSwap(vmmGetBaseTable()); // swap the page table with the base so we can access every piece of memory

    sched_task_t *task = schedGetCurrent(smpID()); // get task context from scheduler

#ifdef K_SYSCALL_DEBUG
    logDbg(LOG_SERIAL_ONLY, "syscall: %s requested %s (0x%x), argument 1 is 0x%x, argument 2 is 0x%x, argument 3 is 0x%x, argument 4 is 0x%x", task->name, syscallNames[rdi], rdi, rsi, rdx, r8, r9);
#endif

    if (rdi < (sizeof(syscallHandlers) / sizeof(void *)))    // check if the syscall is in range
        return syscallHandlers[rdi](rsi, rdx, r8, r9, task); // call the handler
    else
    {
        logError("syscall: %s executed unknown syscall 0x%x", task->name, rdi);
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}

extern void sysretInit();

// init syscall handling
void syscallInit()
{
    // initialise local storage
    cpu_local_storage_t *kernelStorage = (cpu_local_storage_t *)vmmAllocateInitialisationVirtualAddressPage();
    kernelStorage->kernelSyscallStack = vmmAllocateInitialisationVirtualAddressPage();
    kernelStorage->kernel = 1;
    localStorageLoadKernel(kernelStorage);

    cpu_local_storage_t *userspaceStorage = (cpu_local_storage_t *)vmmAllocateInitialisationVirtualAddressPage();
    localStorageLoadUserspace(userspaceStorage);

    sysretInit(); // enable sysret/syscall capability
    logInfo("syscall: enabled syscall/sysret functionality");
}