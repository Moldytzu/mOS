#include <fw/acpi.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <drv/serial.h>
#include <sched/hpet.h>

// NOTE:
// This is not designed to work for anything further than evaluating _PTS and _S5 for shuting down
//

// todo: add ACPI PM timer support

#ifdef K_ACPI_AML

typedef struct
{
    char name[5]; // 5th byte is always null
    uint8_t size; // in bytes
    uint8_t elements;
    uint8_t *contents;
} aml_package_t;

// chapter 20.2 of acpi spec 6.5 (Aug 29 2022)

#define AML_OP_BYTE 0xA
#define AML_OP_WORD 0xB
#define AML_OP_DWORD 0xC
#define AML_OP_QWORD 0xE
#define AML_OP_STRING 0xD

#define AML_OP_ZERO 0
#define AML_OP_ONE 1
#define AML_OP_ONES 0xFF
#define AML_OP_EXTPREFIX 0x5B
#define AML_OP_ALIAS 0x6
#define AML_OP_NAME 0x8
#define AML_OP_SCOPE 0x10
#define AML_OP_CREATEBITFIELD 0x8D
#define AML_OP_CREATEBYTEFIELD 0x8C
#define AML_OP_CREATEDWORDFIELD 0x8A
#define AML_OP_CREATEQWORDFIELD 0x8F
#define AML_OP_CREATEWORDFIELD 0x8B
#define AML_OP_EXTERNAL 0x15
#define AML_OP_METHOD 0x14
#define AML_OP_BREAK 0xA5
#define AML_OP_BREAKPOINT 0xCC
#define AML_OP_CONTINUE 0x9F
#define AML_OP_ELSE 0xA1
#define AML_OP_IF 0xA0
#define AML_OP_NOOP 0xA3
#define AML_OP_NOTIFY 0x86
#define AML_OP_RETURN 0xA4
#define AML_OP_WHILE 0xA2
#define AML_OP_PACKAGE 0x12
#define AML_OP_VARPACKAGE 0x13
// todo: add more operations from 20.2.5.4 acpi spec 6.5 (Aug 29 2022) and so on

uint32_t amlLength = 0;
uint8_t *aml = NULL;

// dump aml contents to serial
void amlDumpSerial()
{
    serialWrite("aml contents dump:");
    for (size_t i = 0; i < amlLength; i++)
        serialWritec(aml[i]);
    serialWritec('\n');
}

// interprets data object at pointed aml (returns its size and puts the value in supplied pointer)
size_t amlInterpretDataObject(uint8_t *aml, uint64_t *value)
{
    if (!aml)
        return 0;

    uint8_t opcode = *aml;
    uint8_t *amlValue = aml++;
    switch (opcode)
    {
    case AML_OP_ZERO:
        *value = 0;
        return 1;

    case AML_OP_ONE:
        *value = 1;
        return 1;

    case AML_OP_ONES:
        *value = ~0;
        return 1;

    case AML_OP_BYTE:
        *value = *amlValue;
        return 1;

    default:
        logError("aml: failed to interpret data object at 0x%p. unexpected opcode 0x%x", aml, opcode);
        return 0;
    };
}

uint8_t *amlGetObject(const char *name)
{
    size_t searchSize = strlen(name);
    for (size_t i = 0; i < amlLength - searchSize; i++)
    {
        if (memcmp(&aml[i], name, searchSize) == 0) // horray found it!
            return &aml[i];
    }

    return NULL;
}

// returns a structure with metadata about the package with the supplied name
aml_package_t amlGetPackage(const char *name)
{
    char *obj = amlGetObject(name); // get object

    aml_package_t package;
    zero(&package, sizeof(aml_package_t));

    if (!obj || !aml)
    {
        logError("aml: failed to find package %s", name);
        return package; // didn't found it
    }

    // the format of a package is
    // 4 bytes name
    // PackageOp
    // PkgLength
    // NumElements
    // PackageElementList

    // parse name
    if (*(obj + 4) != AML_OP_PACKAGE)
    {
        logError("aml: found %s but it is not an package. (detected opcode 0x%x)", name, *(obj + 4));
        return package; // not a package
    }

    memcpy(package.name, obj, 4); // package name always has 4 bytes
    obj += 4;                     // skip name
    obj++;                        // skip opcode

    // parse PkgLength
    uint8_t pkgLength = *obj;
    uint8_t byteDataCount = (pkgLength & 0b11000000) >> 6;
    if (byteDataCount) // todo: handle this
        panick("Can't parse pkgLength! It has multiple byte data");
    else
        package.size = pkgLength & 0b11111;
    obj++; // skip PkgLength

    // parse NumElements
    package.elements = *obj;
    obj++;

    // point to contents
    package.contents = obj;

    return package; // return parsed package
}

// enter _Sx sleep state
bool amlEnterSleepState(uint8_t state)
{
    if (!state || !aml || state > 5)
    {
    fail:
        logError("aml: can't enter unsuported state %d", state);
        return false;
    }

    // fixme: we don't evaluate _TTS and _PTS (may f-up some hardware)

    char object[5];
    sprintf(object, "_S%d_", state);

    aml_package_t sx = amlGetPackage(object); // the _Sx_ package holds the required value to put in type
    if (!sx.size || !sx.elements)
        goto fail;

    logDbg(LOG_ALWAYS, "aml: %s is %d bytes long and has %d elements", object, sx.size, sx.elements);

    uint64_t PM1a_SLP_TYP = 0;
    uint64_t PM1b_SLP_TYP = 0;
    size_t size = 0;
    uint8_t *ptr = sx.contents;

    // we parse those
    size = amlInterpretDataObject(ptr, &PM1a_SLP_TYP);
    ptr += size;
    size = amlInterpretDataObject(ptr, &PM1b_SLP_TYP);

    logDbg(LOG_ALWAYS, "aml: PM1a: %x PM2b: %x", PM1a_SLP_TYP, PM1b_SLP_TYP);

    // make them fit well in the control register
    PM1a_SLP_TYP <<= 10; // PM1_CONTROL_SLP_TYPx starts at bit 10
    PM1b_SLP_TYP <<= 10; // PM1_CONTROL_SLP_TYPx starts at bit 10

    // tell the hardware we want to sleep now
    outw(fadt->PM1aControl, PM1a_SLP_TYP | ACPI_PM1_CONTROL_SLP_EN); // write grabbed type and enable bit

    if (fadt->PM1BControl)
        outw(fadt->PM1BControl, PM1b_SLP_TYP | ACPI_PM1_CONTROL_SLP_EN); // write grabbed type and enable bit

    // wait for things to change
    hpetSleepMillis(1000);

    // if it fails then it didn't work....
    return false;
}

// returns state of acpi enable state
bool amlIsACPIEnabled()
{
    return inw(fadt->PM1aControl) & ACPI_ENABLED;
}

// enables acpi mode if necessary
void amlEnableACPI()
{
    if (!aml)
        return;

    // enable SCIs
    if (!amlIsACPIEnabled()) // already enabled
    {
        logInfo("aml: enabling acpi mode");

        outb(fadt->smiCommand, fadt->acpiEnable);

        // wait for mode to be enabled
        for (int i = 0; i < 1000; i++)
        {
            if (amlIsACPIEnabled())
                return;

            hpetMillis(1);
        }

        logWarn("aml: enabling timed out");
    }

    // set scis for buttons
    uint32_t pm1aEnable = fadt->PM1aEvent + fadt->PM1EventLen / 2;
    uint32_t pm1bEnable = fadt->PM1bEvent + fadt->PM1EventLen / 2;

    outw(pm1aEnable, ACPI_BUTTON_POWER | ACPI_BUTTON_SLEEP);
    outw(pm1bEnable, ACPI_BUTTON_POWER | ACPI_BUTTON_SLEEP);
}

void amlInit()
{
    fadt = (acpi_fadt_t *)acpiGet("FACP", 0); // grab the fadt

    if (!fadt)
    {
        logError("aml: failed to grab FADT (firmware bug?)");
        return;
    }

    vmmMapKernel(fadt, fadt, VMM_ENTRY_RW); // map fadt

    if (fadt->flags & ACPI_FADT_HW_REDUCED_ACPI && rsdp->version >= 5) // check for hardware reduced system (I don't bother with those)
    {
        logError("aml: hardware-reduced system detected. giving up");
        return;
    }

    dsdt = (acpi_dsdt_t *)(fadt->DSDT64 ? fadt->DSDT64 : fadt->DSDT); // use the correct dsdt address

    if (!dsdt)
    {
        logError("aml: failed to grab DSDT (firmware bug?)");
        return;
    }

    vmmMapKernel(dsdt, dsdt, VMM_ENTRY_RW); // map dsdt

    aml = dsdt->aml;
    amlLength = dsdt->header.length - sizeof(acpi_sdt_t);

    logInfo("aml: found dsdt at %p with %d bytes of aml", dsdt, amlLength);

    amlEnableACPI(); // enable acpi mode
}

#endif