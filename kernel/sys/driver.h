#pragma once
#include <sys/sys.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <drv/framebuffer.h>
#include <fw/acpi.h>

// driver (rsi = call, rdx = arg1, r8 = arg2, r9 = arg3)
void driver(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, struct sched_task *task)
{
    if (!task->isDriver && task->id != 1) // unprivileged
        return;

    switch (call)
    {
    case 0: // driver start
        if (!INBOUNDARIES(arg1) && !INBOUNDARIES(arg2))
            return;

        char *path = expandPath(PHYSICAL(arg1), task);
        uint64_t *pid = PHYSICAL(arg2);

        if (!path)
            return;

        elfLoad(path, 0, 0, true); // start the driver
        break;

    case 1: // driver announce
        if (!INBOUNDARIES(arg2))
            return;

        uint64_t *retStruct = PHYSICAL(arg2);

        switch (arg1)
        {
        case 1: // input
            *retStruct = (uint64_t)&drv_type_input_s;
            break;

        case 2: // framebuffer
            *retStruct = (uint64_t)&drv_type_framebuffer_s;
            break;

        default:
            *retStruct = 0;
            break;
        }

        break;

    case 2:           // flush struct updates
        switch (arg1) // type
        {
        case 1: // input
            inputFlush();
            break;
        case 2: // framebuffer
            framebufferFlush();
            break;
        default:
            break;
        }

        break;

    case 3: // redirect idt gate
        if (!INBOUNDARIES(arg1))
            return;

        if (arg2 == 0xF00) // scheduler int
        {
            schedulerHandleDriver((void *)arg1, task->id);
            return;
        }

        idtRedirect((void *)arg1, arg2, task->id); // redirect int arg2 to arg1

        break;
    case 4:                                // reset idt gate
        idtRedirect(NULL, arg1, task->id); // nullify
        break;

    case 5: // get pci header
        if (!INBOUNDARIES(arg1))
            return;

        acpi_pci_header_t **header = (acpi_pci_header_t **)PHYSICAL(arg1);

        volatile acpi_pci_descriptor_t *functions = pciGetFunctions();
        volatile uint64_t num = pciGetFunctionsNum();

        // search for the pci device
        for (int i = 0; i < num; i++)
        {
            if (!functions[i].header)
                continue;

            if (functions[i].header->vendor == arg2 && functions[i].header->device == arg3)
            {
                vmmMap(task->pageTable, functions[i].header, functions[i].header, true, true); // map it
                *header = functions[i].header;                                                 // pass the header

                return;
            }
        }

        break;

    case 6: // identity map
        vmmMap(task->pageTable, (void *)arg1, (void *)arg1, true, true);
        break;

    default:
        break;
    }
}