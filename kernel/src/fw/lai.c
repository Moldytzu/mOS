#include <lai/core.h>
#include <lai/host.h>
#include <mm/blk.h>
#include <mm/vmm.h>
#include <main/panic.h>
#include <misc/logger.h>
#include <fw/acpi.h>
#include <cpu/io.h>

// helper functions for lai
void laihost_log(int level, const char *msg)
{
    logInfo("lai %d: %s", level, msg);
}

__attribute__((noreturn)) void laihost_panic(const char *msg)
{
    panick(msg);
    while (1)
        ;
}

void *laihost_malloc(size_t size)
{
    return blkBlock(size);
}

void *laihost_realloc(void *ptr, size_t newsize, size_t oldsize)
{
    return blkReallocate(ptr, newsize);
}

void *laihost_map(size_t address, size_t count)
{
    if (address % 4096 != 0) // ensure alignment
        address -= address % 4096;

    vmmMap(vmmGetBaseTable(), (void *)address, (void *)address, false, true, false, false);

    return (void *)address;
}

void *laihost_scan(const char *sig, size_t index)
{
    if (strcmp(sig, "DSDT") == 0) // we need it from fadt
        return (void *)(((acpi_fadt_hdr_t *)acpiGet("FACP", 0))->DSDT64);

    return (void *)acpiGet(sig, index);
}

uint8_t laihost_inb(uint16_t port)
{
    return inb(port);
}

uint16_t laihost_inw(uint16_t port)
{
    return inw(port);
}

uint32_t laihost_ind(uint16_t port)
{
    return ind(port);
}

void laihost_outb(uint16_t port, uint8_t val)
{
    outb(port, val);
}

void laihost_outw(uint16_t port, uint16_t val)
{
    outw(port, val);
}

void laihost_outd(uint16_t port, uint32_t val)
{
    outd(port, val);
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outd(0xCF8, address);
    return (uint8_t)((ind(0xCFC) >> ((offset & 2) * 8)) & 0xFF);
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outd(0xCF8, address);
    return (uint16_t)((ind(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outd(0xCF8, address);
    return ind(0xCFC);
}

// stubs, unused for our purposes or for sake of simplicity
uint64_t laihost_timer(void)
{
    return 0;
}

void laihost_sleep(uint64_t ms)
{
}

void laihost_unmap(void *pointer, size_t count)
{
}

void laihost_free(void *ptr, size_t size)
{
}