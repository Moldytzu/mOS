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
        uint64_t fd = openRelativePath(PHYSICAL(arg1), task);
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
        if (!IS_MAPPED(arg2))
            return SYSCALL_STATUS_ERROR;

        uint64_t *ret = PHYSICAL(arg2);

        *ret = (uint64_t)drvRegister(task->id, arg1);

        vmmMap(task->pageTable, (void *)*ret, (void *)*ret, VMM_ENTRY_RW | VMM_ENTRY_USER); // map address

        return SYSCALL_STATUS_OK;

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
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        pcie_ecam_header_t **header = (pcie_ecam_header_t **)PHYSICAL(arg1);

        if (!pcieIsPresent()) // check for pcie availability
        {
            *header = NULL;
            return SYSCALL_STATUS_ERROR;
        }

        pcie_function_descriptor_t *functions = pcieDescriptors();
        size_t num = pcieCountDescriptors();

        // search for the pci device
        for (int i = 0; i < num; i++)
        {
            if (!functions[i].header)
                continue;

            if (functions[i].header->vendor == arg2 && functions[i].header->device == arg3) // check it
            {
                // map it
                vmmMapKernel(functions[i].header, functions[i].header, VMM_ENTRY_RW | VMM_ENTRY_USER | VMM_ENTRY_WRITE_THROUGH);
                vmmMap((void *)task->pageTable, functions[i].header, functions[i].header, VMM_ENTRY_RW | VMM_ENTRY_USER | VMM_ENTRY_WRITE_THROUGH);

                *header = functions[i].header; // pass the header
                return SYSCALL_STATUS_OK;
            }
        }

        return SYSCALL_STATUS_ERROR;

    case 6: // identity map
        vmmMap((void *)task->pageTable, (void *)arg1, (void *)arg1, VMM_ENTRY_RW | VMM_ENTRY_USER);
        return SYSCALL_STATUS_OK;

    case 7:                             // redirect irq to vector
        if (arg2 > 0xFF || arg2 < 0x21) // don't let drivers mess up exceptions and the xapic handler
            return SYSCALL_STATUS_ACCESS_DENIED;

        ioapicRedirectIRQ(arg1, arg2, smpID());
        return SYSCALL_STATUS_OK;

    case 8: // allocate vector
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        uint64_t *retVal = (uint64_t *)arg1;
        *retVal = idtAllocateVector();
        return SYSCALL_STATUS_OK;

    case 9: // deallocate vector
        idtFreeVector(arg1);
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}