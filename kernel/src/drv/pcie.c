#include <drv/pcie.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <misc/logger.h>

pcie_function_descriptor_t *pcieFunctions = NULL;
uint16_t pcieIndex = 0;

void pcieEnumerateECAM(acpi_mcfg_t *mcfg)
{
    if (!mcfg)
        return;

    pcieFunctions = pmmPage();

#ifdef K_PCIE
    size_t entries = (mcfg->header.length - sizeof(acpi_mcfg_t)) / sizeof(acpi_pci_config_t);
    for (int i = 0; i < entries; i++)
    {
        // enumerate each bus
        for (int bus = mcfg->buses[i].startBus; bus < mcfg->buses[i].endBus; bus++)
        {
            uint64_t busBase = mcfg->buses[i].base;
            pcie_ecam_header_t *baseHeader = (pcie_ecam_header_t *)busBase;
            vmmMap(vmmGetBaseTable(), baseHeader, baseHeader, VMM_ENTRY_RW); // map the header

            // check for non-existent bus
            if (baseHeader->device == UINT16_MAX || baseHeader->device == 0)
                continue;

            // enumerate each device
            for (int device = 0; device < 32; device++)
            {
                pcie_ecam_header_t *deviceHeader = (pcie_ecam_header_t *)(busBase + (bus << 20 | device << 15));
                vmmMap(vmmGetBaseTable(), deviceHeader, deviceHeader, VMM_ENTRY_RW); // map the header

                // check for non-existent device
                if (deviceHeader->device == UINT16_MAX || deviceHeader->device == 0)
                    continue;

                // enumerate each function
                for (int function = 0; function < 8; function++)
                {
                    pcie_ecam_header_t *functionHeader = (pcie_ecam_header_t *)(busBase + (bus << 20 | device << 15 | function << 12));

                    // check for non-existent function
                    vmmMap(vmmGetBaseTable(), functionHeader, functionHeader, VMM_ENTRY_RW); // map the header

                    if (functionHeader->device == UINT16_MAX || functionHeader->device == 0)
                        continue;

                    logInfo("pcie: found function %x:%x at %d.%d.%d", functionHeader->vendor, functionHeader->device, bus, device, function);

                    // build the descriptor
                    pcie_function_descriptor_t d;
                    d.bus = bus, d.device = device, d.function = function, d.header = functionHeader;

                    // put it in our list of pci functions (fixme: overflows at 372 descriptors thus we don't have to check anything since it's unlikely to have so many pci functions)
                    pcieFunctions[pcieIndex++] = d;
                }
            }
        }
    }
#endif
}

bool pcieIsPresent()
{
    return pcieIndex && pcieFunctions;
}

pcie_function_descriptor_t *pcieDescriptors()
{
    return pcieFunctions;
}

size_t pcieCountDescriptors()
{
    return pcieIndex;
}