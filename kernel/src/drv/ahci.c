#include <drv/ahci.h>
#include <fw/acpi.h>
#include <misc/logger.h>

pstruct
{
    uint16_t VendorID;
    uint16_t DeviceID;
    uint16_t Command;
    uint16_t Status;
    uint8_t RevisionID;
    uint8_t ProgramInterface;
    uint8_t Subclass;
    uint8_t Class;
    uint8_t CacheLineSize;
    uint8_t LatencyTimer;
    uint8_t HeaderType;
    uint8_t BIST;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t CardBusCISPtr;
    uint16_t SubsystemVendorID;
    uint16_t SubsystemID;
    uint16_t ExpansionRomBaseAddr;
    uint16_t CapabilitiesPtr;
    uint16_t Rsv0;
    uint16_t Rsv1;
    uint16_t Rsv2;
    uint8_t IntreruptLine;
    uint8_t IntreruptPin;
    uint8_t MinGrant;
    uint8_t MaxLatency;
}
drv_pci_header0_t;

drv_pci_header0_t *ahciBase;
acpi_pci_descriptor_t ahciDescriptor;

void ahciInit()
{
    ahciBase = NULL;

    acpi_pci_descriptor_t *pciDescriptors = pciGetFunctions();
    size_t n = pciGetFunctionsNum();

    for (size_t i = 0; i < n; i++)
    {
        if (!pciDescriptors[i].header)
            continue;

        if (pciDescriptors[i].header->class == 1 /*mass storage device*/ && pciDescriptors[i].header->subclass == 6 /*serial ata*/)
        {
            ahciDescriptor = pciDescriptors[i];
            ahciBase = (drv_pci_header0_t *)pciDescriptors[i].header;
            break;
        }
    }

    if(!ahciBase) // didn't find any controller
        return;

    logInfo("ahci: detected controller at %d.%d.%d", ahciDescriptor.bus, ahciDescriptor.device, ahciDescriptor.function);
}