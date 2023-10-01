#include <sys/sys.h>
#include <mm/pmm.h>
#include <drv/framebuffer.h>
#include <drv/drv.h>
#include <drv/input.h>
#include <drv/pcie.h>
#include <elf/elf.h>
#include <cpu/ioapic.h>

#define SYS_DRIVER_TYPE_FRAMEBUFFER 1
#define SYS_DRIVER_TYPE_INPUT 2

// driver (rsi = call, rdx = arg1, r8 = arg2, r9 = arg3)
uint64_t driver(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, sched_task_t *task)
{
    if (!IS_PRIVILEGED) // don't let regular apps use these
        return SYSCALL_STATUS_ACCESS_DENIED;

    switch (call)
    {
    case 0: // driver start
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        // open the relative path
        uint64_t fd = sysOpenRelativePath(PHYSICAL(arg1), task);
        if (!fd)
            return SYSCALL_STATUS_ERROR;

        // gather the full path
        char path[512];
        vfsGetPath(fd, path);

        // close the file descriptor
        vfsClose(fd);

        if (elfLoad(path, 0, 0, true)) // start the driver
            return SYSCALL_STATUS_OK;
        else
            return SYSCALL_STATUS_ERROR;

    case 1: // driver announce

        void *ctx = drvRegister(task->id, arg1);                          // register context
        vmmMap(task->pageTable, ctx, ctx, VMM_ENTRY_RW | VMM_ENTRY_USER); // map its address
        return (uint64_t)ctx;                                             // give it to the driver

    case 2:           // flush struct updates
        switch (arg1) // type
        {
        case SYS_DRIVER_TYPE_INPUT: // input
            inputFlush();
            break;
        case SYS_DRIVER_TYPE_FRAMEBUFFER: // framebuffer
            framebufferFlush();
            break;
        default:
            break;
        }

        return SYSCALL_STATUS_OK;

    case 3: // redirect idt gate
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        if (arg2 > 0xFF || arg2 < 0x21) // don't let drivers mess up exceptions and the xapic handler
            return SYSCALL_STATUS_ERROR;

        idtRedirect((void *)arg1, arg2, task->id); // redirect int arg2 to arg1

        return SYSCALL_STATUS_OK;

    case 4: // reset idt gate
    {
        if (arg1 > 0xFF || arg1 < 0x21) // don't let drivers mess up exceptions and the xapic handler
            return SYSCALL_STATUS_ACCESS_DENIED;

        idtRedirect(NULL, arg1, task->id); // nullify

        return SYSCALL_STATUS_OK;
    }
    case 5: // get pci header

        if (!pcieIsPresent()) // check for pcie availability
            return 0;

        pcie_function_descriptor_t *functions = pcieDescriptors();
        size_t num = pcieCountDescriptors();

        // search for the pci device
        for (int i = 0; i < num; i++)
        {
            if (!functions[i].header)
                continue;

            if (functions[i].header->vendor == arg1 && functions[i].header->device == arg2) // check it
            {
                // map it
                vmmMapKernel(functions[i].header, functions[i].header, VMM_ENTRY_RW | VMM_ENTRY_USER | VMM_ENTRY_WRITE_THROUGH);
                vmmMap((void *)task->pageTable, functions[i].header, functions[i].header, VMM_ENTRY_RW | VMM_ENTRY_USER | VMM_ENTRY_WRITE_THROUGH);

                return (uint64_t)functions[i].header; // return the address
            }
        }

        return 0;

    case 6: // identity map
        vmmMap((void *)task->pageTable, (void *)arg1, (void *)arg1, VMM_ENTRY_RW | VMM_ENTRY_USER);
        return SYSCALL_STATUS_OK;

    case 7:                             // redirect irq to vector
        if (arg2 > 0xFF || arg2 < 0x21) // don't let drivers mess up exceptions and the xapic handler
            return SYSCALL_STATUS_ACCESS_DENIED;

        ioapicRedirectIRQ(arg1, arg2, smpID());
        return SYSCALL_STATUS_OK;

    case 8: // allocate vector
        return idtAllocateVector();

    case 9: // deallocate vector
        idtFreeVector(arg1);
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}