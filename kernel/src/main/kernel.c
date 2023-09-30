#include <misc/utils.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <cpu/fpu.h>
#include <cpu/pic.h>
#include <cpu/control.h>
#include <cpu/smp.h>
#include <cpu/xapic.h>
#include <drv/serial.h>
#include <drv/framebuffer.h>
#include <drv/input.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sched/scheduler.h>
#include <sys/syscall.h>
#include <fw/bootloader.h>
#include <fw/acpi.h>
#include <fs/vfs.h>
#include <fs/initrd.h>
#include <ipc/mailbox.h>
#include <ipc/socket.h>
#include <vt/vt.h>
#include <main/panic.h>
#include <elf/elf.h>
#include <misc/logger.h>

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

    // mailbox_t box;
    // zero(&box, sizeof(mailbox_t));

    // mailCompose(&box, 1, 2, "write xyz", 9);
    // mailCompose(&box, 2, 1, "write abc", 9);
    // mailCompose(&box, 3, 6, "write 123", 9);
    // mailCompose(&box, 4, 3, "write foo", 9);
    // mailCompose(&box, 5, 7, "write bar", 9);

    // mailbox_t *mail = mailReadNext(&box);
    // while (mail)
    // {
    //     logInfo("mail from %d with subject %d says '%s'", mail->sender, mail->subject, mail->message);
    //     mailFree(mail); // deallocate mail
    //     mail = mailReadNext(&box);
    // }

    // while (1)
    //     ;

    if (!elfLoad("/init/init.mx", 0, 0, 0)) // load the init executable
        panick("Failed to load \"init.mx\" from the initrd.");

#ifdef K_FB_DOUBLE_BUFFER
    framebufferInitDoubleBuffer();
#endif
    smpJumpUserspace(); // send all cores to userspace
    schedEnable();
}

// walks the stack using gcc's builtin functions
#define STACK_TRACE_WALK(x) (__builtin_extract_return_addr(__builtin_return_address(x)))

// displays the address if possible
#define PRINT_TRACE_IF_POSSIBLE(x)                              \
    if ((uint64_t)STACK_TRACE_WALK(x) <= 0xFFF8000000000000ULL) \
        hang();                                                 \
    logError("0x%p", STACK_TRACE_WALK(x));

void panick_impl(const char *file, size_t line, const char *msg)
{
#ifdef K_SMP
    xapicNMI(); // send nmis
#endif

    logError("\n\nKernel panic triggered.\n(%s:%d) -> %s\n", file, line, msg);

    // display stack trace
    logError("Stack trace:");
    logError("0x%p <- caller", STACK_TRACE_WALK(0));
    if (!STACK_TRACE_WALK(0))
        hang();

    PRINT_TRACE_IF_POSSIBLE(1);
    PRINT_TRACE_IF_POSSIBLE(2);
    PRINT_TRACE_IF_POSSIBLE(3);
    PRINT_TRACE_IF_POSSIBLE(4);
    PRINT_TRACE_IF_POSSIBLE(5);
    PRINT_TRACE_IF_POSSIBLE(6);
    PRINT_TRACE_IF_POSSIBLE(7);
    PRINT_TRACE_IF_POSSIBLE(8);
    PRINT_TRACE_IF_POSSIBLE(9);
    PRINT_TRACE_IF_POSSIBLE(10);
    PRINT_TRACE_IF_POSSIBLE(11);
    PRINT_TRACE_IF_POSSIBLE(12);
    PRINT_TRACE_IF_POSSIBLE(13);
    PRINT_TRACE_IF_POSSIBLE(14);
    PRINT_TRACE_IF_POSSIBLE(15);
    PRINT_TRACE_IF_POSSIBLE(16);

    // it's very, very unlikely that the stack trace will be this deep
    hang();
}