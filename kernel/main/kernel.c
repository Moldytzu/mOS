#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <drv/serial.h>
#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <drv/input.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <sched/pit.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
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
    struct vfs_node_t *currentNode = vfsNodes();
    do
    {
        if (currentNode->filesystem)
            printks("vfs: found %s%s on %s\n\r", currentNode->filesystem->mountName, currentNode->path, currentNode->filesystem->name);
        currentNode = currentNode->next; // next node
    } while (currentNode);
#endif

    if (!elfLoad("/init/init.mx", 0, 0, 0)) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

    schedulerEnable(); // enable the schduler and jump in userspace
}

void panick(const char *msg)
{
    printk("\n\nA kernel exception has happened.\n%s\n", msg);
    hang();
}