#include <utils.h>
#include <bootloader.h>
#include <idt.h>
#include <gdt.h>
#include <serial.h>
#include <framebuffer.h>
#include <fpu.h>
#include <pmm.h>
#include <vmm.h>
#include <pic.h>
#include <pit.h>
#include <syscall.h>
#include <scheduler.h>
#include <elf.h>

extern void idleTask();
void panick(const char *);

// kernel main, called after init
void kmain()
{
#ifdef K_PMM_DEBUG
    // get the pool total values
    struct mm_pool total = mmGetTotal();

    // display the memory available
    printk("Memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved), total.pageIndex);
#endif

    void *task = mmAllocatePage();             // create an empty page just for the idle task
    memcpy8(task, (void *)idleTask, VMM_PAGE); // copy it

    schedulerAdd("Idle Task", 0, VMM_PAGE, task, VMM_PAGE); // create the idle task

    if (!elfLoad("init.mx")) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

    schedulerEnable(); // enable the schduler and jump in userspace
}

void panick(const char *msg)
{
    printk("\n\nA kernel exception has happened.\n%s\n", msg);
    hang();
}