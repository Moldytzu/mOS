#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <cpu/smp.h>
#include <cpu/lapic.h>
#include <drv/serial.h>
#include <drv/framebuffer.h>
#include <drv/input.h>
#include <drv/ahci.h>
#include <drv/ata.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <fs/initrd.h>
#include <subsys/socket.h>
#include <subsys/vt.h>
#include <main/panic.h>
#include <elf/elf.h>
#include <misc/logger.h>
#include <lai/helpers/pm.h>
#include <lai/helpers/sci.h>

// kernel main, called after init
void kmain()
{
#ifdef K_PMM_DEBUG
    // get the pool total values
    pmm_pool_t total = pmmTotal();

    // display the memory available
    logDbg(LOG_SERIAL_ONLY, "memory: total= %d MB; available= %d MB; used= %d MB; bitmap reserved= %d KB;", (total.available + total.used) / 1024 / 1024, (total.available) / 1024 / 1024, (total.used) / 1024 / 1024, (total.bitmapBytes) / 1024);
#endif

#ifdef K_VFS_DEBUG
    // get all the nodes
    struct vfs_node_t *currentNode = vfsNodes();
    do
    {
        if (currentNode->filesystem)
           logDbg(LOG_SERIAL_ONLY, "vfs: found %s%s on %s", currentNode->filesystem->mountName, currentNode->path, currentNode->filesystem->name);
        currentNode = currentNode->next; // next node
    } while (currentNode);
#endif

#if 1
    ataInit();
#endif

#if 0
    ahciInit();

    while (1);
#endif

    if (!elfLoad("/init/init.mx", 0, 0, 0)) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

#ifdef K_FB_DOUBLE_BUFFER
    framebufferInitDoubleBuffer();
#endif
    smpJumpUserspace(); // send all cores to userspace
    schedEnable();
}

void panick_impl(const char *file, size_t line, const char *msg)
{
#ifdef K_SMP
    lapicNMI(); // send nmis
#endif

    logError("\n\nKernel panic triggered.\n(%s:%d) -> %s\n", file, line, msg);

#ifdef K_PANIC_REBOOT
    for (volatile size_t i = 0; i < 0xFFFFFFF; i++)
        ; // wait a bit

    acpiReboot();
#endif

    hang();
}