#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <drv/serial.h>
#include <drv/ps2.h>
#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/pit.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <subsys/input.h>
#include <subsys/socket.h>
#include <main/panic.h>
#include <elf/elf.h>

void panick(const char *);

// kernel main, called after init
void kmain()
{
#ifdef K_PMM_DEBUG
    // get the pool total values
    pmm_pool_t total = pmmTotal();

    // display the memory available
    printks("memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n\r", toMB(total.available + total.used), toMB(total.available), toMB(total.used), toKB(total.bitmapBytes), total.lastPageIndex);
#endif

#ifdef K_VFS_DEBUG
    // get all the nodes
    struct vfs_node *currentNode = vfsNodes();
    do
    {
        if (currentNode->filesystem)
            printks("vfs: found %s%s on %s\n\r", currentNode->filesystem->mountName, currentNode->path, currentNode->filesystem->name);
        currentNode = currentNode->next; // next node
    } while (currentNode);
#endif

    if (!elfLoad("/init/init.mx", 0, 0)) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

    schedulerEnable(); // enable the schduler and jump in userspace
}

void exceptionHandler(struct idt_intrerrupt_stack *stack)
{
    vmmSwap(vmmGetBaseTable()); // swap to the base table
    if (stack->cs == 0x23)      // userspace
    {
        struct sock_socket *initSocket = sockGet(1);
        
        if(initSocket == NULL)
            printks("\n\r%s crashed! Terminating it.\n\r", schedulerGetCurrent()->name);
        else 
            sockAppend(initSocket, "crash <apllication>", strlen("crash <apllication>")); // announce that the application has crashed
        
        schedulerKill(schedulerGetCurrent()->id); // terminate the task
        schedulerSchedule(stack);                 // schedule next task
        return;
    }

    printk("\nCATCHED EXCEPTION. INFORMATION:\n");
    printk("RIP=0x%p CS=0x%p RFLAGS=0x%p RSP=0x%p SS=0x%p", stack->rip, stack->cs, stack->rflags, stack->rsp, stack->ss);
    panick("Generic x86_64 exception catched.");
}

void panick(const char *msg)
{
    framebufferClear(0);
    printk("\n\nA kernel exception has happened.\n%s\n", msg);
    hang();
}