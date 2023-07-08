#pragma once
#include <misc/utils.h>
#include <fw/acpi.h>

pstruct
{
    uint16_t vendor;
    uint16_t device;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t programInterface;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
}
pcie_ecam_header_t;

pstruct
{
    pcie_ecam_header_t *header;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
}
pcie_function_descriptor_t;

bool pcieIsPresent();
void pcieEnumerateECAM(acpi_mcfg_t *mcfg);
pcie_function_descriptor_t *pcieDescriptors();
size_t pcieCountDescriptors();