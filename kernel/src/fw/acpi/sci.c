#include <fw/acpi.h>
#include <cpu/io.h>
#include <cpu/xapic.h>
#include <cpu/ioapic.h>
#include <cpu/idt.h>
#include <subsys/socket.h>
#include <misc/logger.h>
#include <mm/vmm.h>

// gets last SCI event
uint32_t acpiGetSCIEvent()
{
    // read status
    uint32_t pm1aStatus = inw(fadt->PM1aEvent);
    uint32_t pm1bStatus = inw(fadt->PM1bEvent);

    // reset status
    outw(fadt->PM1aEvent, pm1aStatus);
    outw(fadt->PM1bEvent, pm1bStatus);

    // return values
    return pm1aStatus | pm1bStatus;
}

// handles SCI interrupts
void sciHandler(idt_intrerrupt_error_stack_t *stack)
{
    vmmSwap(vmmGetBaseTable()); // swap to kernel table

    xapicEOI(); // send end of interrupt

    uint32_t event = acpiGetSCIEvent(); // get event

    logDbg(LOG_SERIAL_ONLY, "SCI event 0x%x", event);

    // give the init system the information
    struct sock_socket *initSocket = sockGet(1);
    if (!initSocket)
        return;

    if (event & ACPI_BUTTON_POWER)
        sockAppend(initSocket, "acpi_power", 10);

    if (event & ACPI_BUTTON_SLEEP)
        sockAppend(initSocket, "acpi_sleep", 10);
}

extern void sciEntry();

// install SCI interrupt handler
void acpiInstallSCIInterrupt()
{
    logInfo("aml: routing %d to SCI interrupt handler", fadt->sciIntrerrupt);

    uint16_t vector = idtAllocateVector();
    idtSetGate(sciEntry, vector);
    ioapicRedirectIRQ(fadt->sciIntrerrupt, vector, 0);
}