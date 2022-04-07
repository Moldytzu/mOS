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

extern void idleTask();

// kernel main, called after init
void kmain()
{
    // get the pool total values
    struct mm_pool total = mmGetTotal();

    // display the memory available
    printk("Memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved), total.pageIndex);

    void *task = mmAllocatePage();            // create an empty page just for the idle task
    memcpy(task, (void *)idleTask, VMM_PAGE); // copy it

    schedulerAdd("Idle Task", 0, VMM_PAGE, task, VMM_PAGE);   // create the idle task
    schedulerAdd("Idle Task 2", 0, VMM_PAGE, task, VMM_PAGE); // create the idle task
    schedulerEnable();                                        // enable the schduler and jump in userspace
}

void panick(const char *msg)
{
    printk("\n\nA kernel exception happened.\n%s\n", msg);
    hang();
}