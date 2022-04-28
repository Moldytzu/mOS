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
#include <vfs.h>

void panick(const char *);

// kernel main, called after init
void kmain()
{
#ifdef K_PMM_DEBUG
    // get the pool total values
    struct mm_pool total = mmGetTotal();

    // display the memory available
    printks("memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n\r", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved), total.pageIndex);
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

    if (!elfLoad("/init/init.mx")) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

    schedulerEnable(); // enable the schduler and jump in userspace
}

void panick(const char *msg)
{
    printk("\n\nA kernel exception has happened.\n%s\n", msg);
    hang();
}