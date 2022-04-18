#include <lai.h>

void laihost_log(int level, const char *msg)
{

}

__attribute__((noreturn)) void laihost_panic(const char *msg)
{
    __builtin_unreachable();
}

void *laihost_malloc(size_t size)
{

}

void *laihost_realloc(void *oldptr, size_t newsize, size_t oldsize)
{

}

void laihost_free(void *ptr, size_t size)
{

}

void *laihost_map(size_t address, size_t count)
{

}

void laihost_unmap(void *pointer, size_t count)
{

}

void *laihost_scan(const char *sig, size_t index)
{

}

void laihost_outb(uint16_t port, uint8_t val)
{

}

void laihost_outw(uint16_t port, uint16_t val)
{

}

void laihost_outd(uint16_t port, uint32_t val)
{

}

uint8_t laihost_inb(uint16_t port)
{

}

uint16_t laihost_inw(uint16_t port)
{

}

uint32_t laihost_ind(uint16_t port)
{

}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val)
{

}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val)
{

}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val)
{

}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{

}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{

}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{

}

void laihost_sleep(uint64_t ms)
{

}

uint64_t laihost_timer(void)
{

}

void laihost_handle_amldebug(lai_variable_t *var)
{

}

void lai_set_acpi_revision(int revision)
{
    
}