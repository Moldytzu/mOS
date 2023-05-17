#include <drv/ahci.h>
#include <fw/acpi.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <sched/time.h>

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

pstruct
{
    uint32_t commandListBaseLow;
    uint32_t commandListBaseHigh;
    uint32_t fisBaseLow;
    uint32_t fisBaseHigh;
    uint32_t interruptStatus;
    uint32_t interruptEnable;
    uint32_t commandStatus;
    uint32_t reserved;
    uint32_t taskFileData;
    uint32_t signature;
    uint32_t sataStatus;
    uint32_t sataControl;
    uint32_t sataError;
    uint32_t sataActive;
    uint32_t commandIssue;
    uint32_t sataNotification;
    uint32_t fisSwitchControl;
    uint32_t deviceSleep;
    uint32_t reserved2;
    uint32_t vendorSpecific;
}
ahci_port_t;

void *ahciABAR;                       // ahci's pci bar 5
drv_pci_header0_t *ahciBase;          // base pointer to pci header
acpi_pci_descriptor_t ahciDescriptor; // descriptor that houses the header and the address
bool portImplemented[32];             // stores which ports are present
uint8_t portsImplemented = 0;         // stores how many ports are present
uint32_t commandSlots = 0;            // stores how many command slots the ahci hba supports

// read from abar at offset
uint32_t ahciRead(uint32_t offset)
{
    return *(uint32_t *)((uint64_t)ahciABAR + offset);
}

// write to abar at offset
void ahciWrite(uint32_t offset, uint32_t data)
{
    *(uint32_t *)((uint64_t)ahciABAR + offset) = data;
}

// get pointer base address as shown by bits set in PI
ahci_port_t *ahciPort(uint8_t bit)
{
    return (ahci_port_t *)((uint64_t)ahciABAR + 0x100 + 0x80 * bit);
}

// force a port in idle
void ahciPortStop(ahci_port_t *port)
{
    // disable fis and disable commands
    port->commandIssue &= ~(0x0);  // unset start bit (bit 0)
    port->commandIssue &= ~(0x10); // unset fis receive enable (bit 4)

    // spec recommends waiting 500 miliseconds for this to happen and if after that time period the bits weren't disable we should perform a hba reset or a port reset
    // for sake of speed and simplicity we just have a while loop

    // wait for fis running and command running to be clear
    while (port->commandIssue & 0x4000 || port->commandIssue & 0x8000)
        timeSleepMilis(1);
}

// determine if a port is idle
bool ahciPortIsIdle(ahci_port_t *port)
{
    return !(port->commandIssue & 0) && !(port->commandIssue & 0x8000) && !(port->commandIssue & 0x10); // a port is idle if fis receive enable, command list running and start are clear
}

void ahciInit()
{
    ahciBase = NULL;
    zero(portImplemented, sizeof(portImplemented));

    acpi_pci_descriptor_t *pciDescriptors = pciGetFunctions();
    size_t n = pciGetFunctionsNum();

    // probe to get ahci base
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

    if (!ahciBase) // didn't find any controller
        return;

    logInfo("ahci: detected controller at %d.%d.%d", ahciDescriptor.bus, ahciDescriptor.device, ahciDescriptor.function);

    // enable access to internal registers
    ahciBase->Command |= 0b10000010111;                                                    // enable i/o space, enable memory space, enable bus mastering, enable memory write and invalidate and disable intrerrupts
    ahciABAR = (void *)(uint64_t)(ahciBase->BAR5 & 0xFFFFE000);                            // get base address
    vmmMap(vmmGetBaseTable(), ahciABAR, ahciABAR, VMM_ENTRY_CACHE_DISABLE | VMM_ENTRY_RW); // map base

    logDbg(LOG_ALWAYS, "ahci: abar is at %p", ahciABAR);

    // don't allow 32 bit only ahci controllers
    uint32_t cap = ahciRead(0x0); // read host capabilities

    if (!(cap & 0x80000000)) // if controller doesn't support 64 bit addressing give up
        return;

    // transfer ownership to operating system
    uint32_t bohc = ahciRead(0x28);     // read bios/os handoff control and status
    if (bohc & 0x1)                     // bios owns hba
        ahciWrite(0x28, bohc | 0b1000); // set os ownership change bit

    logDbg(LOG_ALWAYS, "ahci: waiting for ownership to change");

    while (ahciRead(0x28) & 0x1) // wait for the bios to transfer ownership
        timeSleepMilis(1);

    // INITIALISATION (ahci 1.3.1 spec page 112)

    // 1. Indicate that system software is AHCI aware by setting GHC.AE to ‘1’.

    uint32_t control = ahciRead(0x4);     // enable ahci mode
    ahciWrite(0x4, control | 0x80000000); // set ahci enable bit

    // 2. Determine which ports are implemented by the HBA, by reading the PI register

    uint32_t pi = ahciRead(0xC);    // read ports implemented
    for (size_t i = 0; i < 32; i++) // each bit represents a port's presence
    {
        if (!(pi & 1))
            continue;

        pi >>= 1;

        portImplemented[i] = true;
        portsImplemented++;
    }

    logDbg(LOG_ALWAYS, "ahci: %d ports are implemented", portsImplemented);

    // 3. Ensure that the controller is not in the running state by reading and examining each implemented port’s PxCMD register

    // todo: we don't check for the controller to be idle. we probably want to do that

    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);
        if(!ahciPortIsIdle(port))
        {
            ahciPortStop(port);
            logDbg(LOG_ALWAYS, "ahci: port %d wasn't idle so we forced it to be.", p);
        }
    }

    // 4. Determine how many command slots the HBA supports, by reading CAP.NCS

    commandSlots = (ahciRead(0x0) >> 8) & 0x1F;

    logDbg(LOG_ALWAYS, "ahci: %d command slots supported", commandSlots);

    // 5. For each implemented port, system software shall allocate memory

    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);
        
        uint64_t address = (uint64_t)pmmPage(); // todo: i'm not sure how much memory should I allocate
        port->commandListBaseLow = address & 0xFFFFFFFF;
        port->commandListBaseHigh = (address >> 32) & 0xFFFFFFFF;
    
        address = (uint64_t)pmmPage(); // todo: neither here
        port->fisBaseLow = address & 0xFFFFFFFF;
        port->fisBaseHigh = (address >> 32) & 0xFFFFFFFF;
    }

    // 6. For each implemented port, clear the PxSERR register, by writing ‘1s’ to each implemented bit location.
    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);
        port->sataError = 0xFFFFFFFF;
    }

    // 7. todo: Determine which events should cause an interrupt, and set each implemented port’s PxIE register with the appropriate enables.

    logInfo("ahci: initialised controller");
}