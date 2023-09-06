#include <drv/ahci.h>
#include <drv/serial.h>
#include <drv/pcie.h>
#include <misc/logger.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <sched/time.h>
#include <fs/vfs.h>

#ifdef K_AHCI

#define GEN_HANDLER_READ(port)                                                      \
    void aport##port##read(void *buffer, uint64_t sector, uint64_t count)           \
    {                                                                               \
        void *tmp = pmmPage();                                                      \
        ahciPortRead(ahciPort(port), sector, sector >> 32, count, (uint16_t *)tmp); \
        memcpy(buffer, tmp, count *VFS_SECTOR);                                     \
        pmmDeallocate(tmp);                                                         \
    }

uint64_t ahciBase;                         // ahci's pci bar 5
drv_pci_header0_t *ahciHeader;             // base pointer to pci header
pcie_function_descriptor_t ahciDescriptor; // descriptor that houses the header and the address
bool portImplemented[32];                  // stores which ports are present
uint8_t portsImplemented = 0;              // stores how many ports are present
uint32_t commandSlots = 0;                 // stores how many command slots the ahci hba supports

// find a free command list slot
int ahciPortSlot(ahci_port_t *port)
{
    // if not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < commandSlots; i++)
    {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    return -1;
}

// read from ahciBase at offset
ifunc uint32_t ahciRead(uint32_t offset)
{
    return *(volatile uint32_t *)(ahciBase + offset);
}

// write to ahciBase at offset
ifunc void ahciWrite(uint32_t offset, uint32_t data)
{
    *(volatile uint32_t *)(ahciBase + offset) = data;
}

// get pointer base address as shown by bits set in PI
ifunc ahci_port_t *ahciPort(uint8_t bit)
{
    return (ahci_port_t *)(ahciBase + 0x100 + 0x80 * bit);
}

// start command engine
void ahciPortStart(ahci_port_t *port)
{
    // Set FRE (bit 4) and ST (bit 0)
    port->cmd |= (HBA_PxCMD_FRE | HBA_PxCMD_ST);
}

// stop command engine
void ahciPortStop(ahci_port_t *port)
{
    // clear ST (bit 0) and FRE (bit 4)
    port->cmd &= ~(HBA_PxCMD_ST | HBA_PxCMD_FRE);

    // wait until FR (bit 14), CR (bit 15) are cleared
    while (port->cmd & HBA_PxCMD_FR || port->cmd & HBA_PxCMD_CR)
        ;
}

// send command
void ahciPortSendCommand(ahci_port_t *port, int slot)
{
    ahciPortStart(port);

    // wait for the port to be idle
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)))
        pause();

    port->ci = 1 << slot; // issue command

    // wait for completion
    while (1)
        if ((port->ci & (1 << slot)) == 0)
            break;

    ahciPortStop(port);
}

ahci_tbl_t *ahciPrepareCommandHeader(ahci_port_t *port, uint16_t *buffer, size_t size, uint16_t *slot)
{
    port->is = (uint32_t)-1;    // clear interrupts
    *slot = ahciPortSlot(port); // allocate a slot
    if (*slot == -1)            // failure
        return false;

    ahci_command_header_t *cmdheader = (ahci_command_header_t *)port->clb;
    cmdheader += *slot;
    cmdheader->cfl = sizeof(ahci_fis_h2d_t) / sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;                                           // Read from device
    cmdheader->prdtl = (uint16_t)(size / 512);                  // PRDT entries count

    ahci_tbl_t *cmdtbl = (ahci_tbl_t *)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(ahci_tbl_t) + (cmdheader->prdtl - 1) * sizeof(ahci_prdt_t));

    // 1 sector per prdt
    int i = 0;
    for (; i < cmdheader->prdtl - 1; i++)
    {
        // point the data base address to the buffer
        cmdtbl->prdt_entry[i].dba = (uint32_t)(uint64_t)buffer;
        cmdtbl->prdt_entry[i].dbau = (uint32_t)((uint64_t)buffer >> 32);
        cmdtbl->prdt_entry[i].dbc = 512 - 1; // 1 sector
        cmdtbl->prdt_entry[i].i = 1;

        // increase buffer
        buffer += 256;
        size -= 512;
    }
    cmdtbl->prdt_entry[i].dba = (uint32_t)(uint64_t)buffer;
    cmdtbl->prdt_entry[i].dbau = (uint32_t)((uint64_t)buffer >> 32);
    cmdtbl->prdt_entry[i].dbc = align(size, 512) - 1;
    cmdtbl->prdt_entry[i].i = 1;

    return cmdtbl;
}

// read using dma from a port
bool ahciPortRead(ahci_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buffer)
{
    uint16_t slot;
    ahci_tbl_t *cmdtbl = ahciPrepareCommandHeader(port, buffer, count * 512, &slot);
    if (!cmdtbl)
        return false;

    // Setup command
    ahci_fis_h2d_t *cmdfis = (ahci_fis_h2d_t *)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;   // host -> device
    cmdfis->c = 1;                         // is command
    cmdfis->command = ATA_CMD_READ_DMA_EX; // read with dma
    cmdfis->device = 1 << 6;               // LBA mode

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    ahciPortSendCommand(port, slot); // send command

    return true;
}

bool ahciPortIdentify(ahci_port_t *port, void *buffer)
{
    uint16_t slot;
    ahci_tbl_t *cmdtbl = ahciPrepareCommandHeader(port, buffer, 512, &slot);
    if (!cmdtbl)
        return false;

    // Setup command
    ahci_fis_h2d_t *cmdfis = (ahci_fis_h2d_t *)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;       // host -> device
    cmdfis->c = 1;                             // is command
    cmdfis->command = ATA_CMD_IDENTIFY_DEVICE; // identify device
    cmdfis->device = 0;

    ahciPortSendCommand(port, slot); // send command

    return true;
}

// allocate memory for each port
void ahciPortAllocate(ahci_port_t *port, int portno)
{
    ahciPortStop(port); // stop command engine

    // allocate command list buffer
    void *page = pmmPage();
    port->clb = (uint64_t)page;
    port->clbu = (uint64_t)page << 32;

    // allocate fis
    page = pmmPage();
    port->fb = (uint64_t)page;
    port->fbu = (uint64_t)page << 32;

    // allocate each command header
    ahci_command_header_t *cmdheader = (ahci_command_header_t *)(port->clb);
    for (int i = 0; i < commandSlots; i++)
    {
        page = pmmPage();
        cmdheader[i].prdtl = 8;
        cmdheader[i].ctba = (uint64_t)page;
        cmdheader[i].ctbau = (uint64_t)page << 32;
    }

    ahciPortStart(port); // start command engine
}

GEN_HANDLER_READ(0)
GEN_HANDLER_READ(1)
GEN_HANDLER_READ(2)
GEN_HANDLER_READ(3)
GEN_HANDLER_READ(4)
GEN_HANDLER_READ(5)
GEN_HANDLER_READ(6)
GEN_HANDLER_READ(7)
GEN_HANDLER_READ(8)
GEN_HANDLER_READ(9)
GEN_HANDLER_READ(10)
GEN_HANDLER_READ(11)
GEN_HANDLER_READ(12)
GEN_HANDLER_READ(13)
GEN_HANDLER_READ(14)
GEN_HANDLER_READ(15)
GEN_HANDLER_READ(16)
GEN_HANDLER_READ(17)
GEN_HANDLER_READ(18)
GEN_HANDLER_READ(19)
GEN_HANDLER_READ(20)
GEN_HANDLER_READ(21)
GEN_HANDLER_READ(22)
GEN_HANDLER_READ(23)
GEN_HANDLER_READ(24)
GEN_HANDLER_READ(25)
GEN_HANDLER_READ(26)
GEN_HANDLER_READ(27)
GEN_HANDLER_READ(28)
GEN_HANDLER_READ(29)
GEN_HANDLER_READ(30)
GEN_HANDLER_READ(31)

void (*ahciReads[])(void *, uint64_t, uint64_t) = {
    aport0read, aport1read, aport2read, aport3read, aport4read, aport5read, aport6read, aport7read, aport8read, aport9read, aport10read, aport11read, aport12read, aport13read, aport14read, aport15read, aport16read, aport17read, aport18read, aport19read, aport20read, aport21read, aport22read, aport23read, aport24read, aport25read, aport26read, aport27read, aport28read, aport29read, aport30read, aport31read};

void ahciInit()
{
    ahciHeader = NULL;
    zero(portImplemented, sizeof(portImplemented));

    pcie_function_descriptor_t *pciDescriptors = pcieDescriptors();
    size_t n = pcieCountDescriptors();

    // probe to find the controller
    for (size_t i = 0; i < n; i++)
    {
        if (!pciDescriptors[i].header)
            continue;

        if (pciDescriptors[i].header->class == 1 /*mass storage device*/ && pciDescriptors[i].header->subclass == 6 /*serial ata*/)
        {
            ahciDescriptor = pciDescriptors[i];
            ahciHeader = (drv_pci_header0_t *)pciDescriptors[i].header;
            break;
        }
    }

    if (!ahciHeader) // didn't find any controller
        return;

    logInfo("ahci: detected controller at %d.%d.%d", ahciDescriptor.bus, ahciDescriptor.device, ahciDescriptor.function);

    // enable access to internal registers
    ahciHeader->Command |= 0b10000010111;                                                     // enable i/o space, enable memory space, enable bus mastering, enable memory write and invalidate and disable intrerrupts
    ahciBase = (uint64_t)(ahciHeader->BAR5 & 0xFFFFE000);                                     // get base address
    vmmMapKernel((void *)ahciBase, (void *)ahciBase, VMM_ENTRY_CACHE_DISABLE | VMM_ENTRY_RW); // map base

    logDbg(LOG_ALWAYS, "ahci: ahciBase is at %p", ahciBase);

    // don't allow 32 bit only ahci controllers
    uint32_t cap = ahciRead(0x0); // read host capabilities

    if (!(cap & 0x80000000)) // if controller doesn't support 64 bit addressing give up
        return;

    // transfer ownership to operating system
    uint32_t bohc = ahciRead(0x28); // read bios/os handoff control and status
    if (bohc & 0x1)                 // bios owns hba
    {
        ahciWrite(0x28, bohc | 0b1000); // set os ownership change bit

        logDbg(LOG_ALWAYS, "ahci: waiting for ownership to change");

        size_t timeout = 1000;
        while ((ahciRead(0x28) & 0x1) && --timeout) // wait for the bios to transfer ownership
            timeSleepMilis(1);

        if (!timeout)
            logWarn("ahci: ownership change timed out (maybe firmware bug?)");
    }

    // INITIALISATION (ahci 1.3.1 spec page 112)

    // 1. Indicate that system software is AHCI aware by setting GHC.AE to ‘1’.
    ahciWrite(0x4, ahciRead(0x4) | 0x80000000); // set ahci enable bit

    // 2. Determine which ports are implemented by the HBA, by reading the PI register
    uint32_t pi = ahciRead(0xC);    // read ports implemented
    for (size_t i = 0; i < 32; i++) // each bit represents a port's presence
    {
        if (!(pi & 1))
            continue;

        pi >>= 1;

        if (ahciPort(i)->sig != SATA_SIG_ATA) // we support only sata devices
            continue;

        portImplemented[i] = true;
        portsImplemented++;
    }

    logDbg(LOG_ALWAYS, "ahci: %d sata port(s) implemented", portsImplemented);

    // 4. Determine how many command slots the HBA supports, by reading CAP.NCS
    commandSlots = (ahciRead(0x0) >> 8) & 0x1F;

    logDbg(LOG_ALWAYS, "ahci: %d command slots supported", commandSlots);

    // prepare ports for commands
    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);

        ahciPortStop(port);        // 3. Ensure that the controller is not in the running state by reading and examining each implemented port’s PxCMD register
        ahciPortAllocate(port, p); // 5. For each implemented port, system software shall allocate memory
        port->serr = 0xFFFFFFFF;   // 6. For each implemented port, clear the PxSERR register, by writing ‘1s’ to each implemented bit location.
    }

    // 7. Determine which events should cause an interrupt, and set each implemented port’s PxIE register with the appropriate enables. (todo: we don't want this at the moment, maybe when we create an I/O scheduler)

    // INITIALISATION complete
    logInfo("ahci: initialised controller");

    // identify all devices
    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);
        ata_identify_device_packet_t *response = pmmPage();
        ahciPortIdentify(port, response);

        uint16_t *raw = (uint16_t *)response;

        printks("ahci port %d identification packet: ", p);
        for (int i = 0; i < 256; i++)
            printks("%x ", raw[i]);
        printks(" model number: ");
        for (int i = 0; i < 20; i += 2)
            printks("%c%c", response->modelNumber[i + 1], response->modelNumber[i]);
        printks("\n");
    }

    // map all drives in the vfs
    for (int p = 0; p < 32; p++)
    {
        if (portImplemented[p] == false)
            continue;

        ahci_port_t *port = ahciPort(p);

        // register the drive in vfs
        vfs_drive_t drive;
        zero(&drive, sizeof(drive));
        drive.interface = "ahci";
        drive.friendlyName = pmmPage();
        drive.read = ahciReads[p];
        const char *str = to_string(p);
        memcpy((void *)drive.friendlyName, str, strlen(str));

        vfs_mbr_t *firstSector = pmmPage();
        ahciReads[p](firstSector, 0, 1);

        if (vfsCheckMBR(firstSector)) // check if mbr is valid
        {
            int part = 0;
            for (int i = 0; i < 4; i++) // parse all the partitions
            {
                vfs_mbr_partition_t *partition = &firstSector->partitions[i];
                if (!partition->startSector)
                    continue;

                vfs_partition_t *vfsPart = &drive.partitions[part++];
                vfsPart->startLBA = partition->startSector;
                vfsPart->endLBA = partition->startSector + partition->sectors;
                vfsPart->sectors = partition->sectors;
            }
        }

        vfsAddDrive(drive);
    }
}

#endif