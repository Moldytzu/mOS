#include <fw/acpi.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpu/io.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <drv/serial.h>

// NOTE:
// This is not designed to work for anything further than evaluating _PTS and _S5 for shuting down
//

// chapter 20.2 of acpi spec 6.5 (Aug 29 2022)

#define AML_DATA_PREFIX_BYTE 0xA
#define AML_DATA_PREFIX_WORD 0xB
#define AML_DATA_PREFIX_DWORD 0xC
#define AML_DATA_PREFIX_QWORD 0xE
#define AML_DATA_PREFIX_STRING 0xD

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
// todo: add more operations from 20.2.5.4 and so on

uint32_t amlLength;
uint8_t *aml;

// dump aml contents to serial
void amlDumpSerial()
{
    serialWrite("aml contents dump:");
    for (size_t i = 0; i < amlLength; i++)
        serialWritec(aml[i]);
    serialWritec('\n');
}

uint8_t *amlGetPackage(const char *namespaceName)
{
    size_t searchSize = strlen(namespaceName);
    for (size_t i = 0; i < amlLength - searchSize; i++)
    {
        if (memcmp(&aml[i], namespaceName, searchSize) == 0) // horray found it!
            return &aml[i];
    }

    return NULL; // nah
}

void amlInit()
{
    fadt = (acpi_fadt_t *)acpiGet("FACP", 0);                         // grab the fadt
    dsdt = (acpi_dsdt_t *)(fadt->DSDT64 ? fadt->DSDT64 : fadt->DSDT); // use the correct dsdt address

    aml = dsdt->aml;
    amlLength = dsdt->header.length - sizeof(acpi_sdt_t);

    logInfo("acpi: found dsdt at %p with %d bytes of aml", dsdt, amlLength);

    amlDumpSerial();

    serialWrite("_S5: ");

    uint8_t *s5 = amlGetPackage("_S5");
    for (size_t i = 0; i < 10; i++)
        serialWritec(s5[i]);
}