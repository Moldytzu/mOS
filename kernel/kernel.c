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
    printk("Memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB; pool count= %d;\n", toMB(total.total), toMB(total.available), toMB(total.used), toKB(total.bitmapReserved), total.pageIndex);
#endif

#ifdef K_VFS_DEBUG
    // get all the nodes
    struct vfs_node *nodes = vfsNodes();
    for(int i = 0; i < 0xFF; i++) // iterate over each node
    {
        if(nodes[i].filesystem)
            printk("path %s%s on %s \n",nodes[i].filesystem->mountName,nodes[i].path,nodes[i].filesystem->name);
    }
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