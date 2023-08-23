#include <fw/acpi.h>
#include <cpu/io.h>

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