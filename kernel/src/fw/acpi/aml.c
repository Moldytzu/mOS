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

typedef struct
{
    char name[5]; // 5th byte is always null
    uint8_t size; // in bytes
    uint8_t elements;
    uint8_t *contents;
} aml_package_t;

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
#define AML_OP_VARPACKAGE 0x13
// todo: add more operations from 20.2.5.4 acpi spec 6.5 (Aug 29 2022) and so on

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

// returns a structure with metadata about the package with the supplied name
aml_package_t amlGetPackage(const char *name)
{
    char *ptr = NULL;

    size_t searchSize = strlen(name);
    for (size_t i = 0; i < amlLength - searchSize; i++)
    {
        if (memcmp(&aml[i], name, searchSize) == 0) // horray found it!
            ptr = &aml[i];
    }

    aml_package_t package;
    zero(&package, sizeof(aml_package_t));

    if (!ptr)
        return package; // didn't found it

    // the format of a package is
    // 4 bytes name
    // PackageOp
    // PkgLength
    // NumElements
    // PackageElementList

    // parse name
    if (*(ptr + 4) != AML_OP_PACKAGE)
        return package; // not a package

    memcpy(package.name, ptr, 4); // package name always has 4 bytes
    ptr += 4;                     // skip name
    ptr++;                        // skip opcode

    // parse PkgLength
    uint8_t pkgLength = *ptr;
    uint8_t byteDataCount = (pkgLength & 0b11000000) >> 6;
    if (byteDataCount)
        panick("Can't parse pkgLength! It has multiple byte data");
    else
        package.size = pkgLength & 0b11111;
    ptr++; // skip PkgLength

    // parse NumElements
    package.elements = *ptr;
    ptr++;

    // point to contents
    package.contents = ptr;

    return package; // return parsed package
}

void amlInit()
{
    fadt = (acpi_fadt_t *)acpiGet("FACP", 0);                         // grab the fadt
    dsdt = (acpi_dsdt_t *)(fadt->DSDT64 ? fadt->DSDT64 : fadt->DSDT); // use the correct dsdt address

    aml = dsdt->aml;
    amlLength = dsdt->header.length - sizeof(acpi_sdt_t);

    logInfo("acpi: found dsdt at %p with %d bytes of aml", dsdt, amlLength);

    amlDumpSerial();

    aml_package_t s5 = amlGetPackage("_S5");
    logInfo("_S5_ is %d bytes long and has %d elements", s5.size, s5.elements);

    // while (1)
    ;
}