#include <acpi.h>
#include <heap.h>
#include <vmm.h>
#include <bootloader.h>

void laihost_log(int level, const char *msg)
{
    printk("lai msg %d: %s\n",level,msg);
}

__attribute__((noreturn)) void laihost_panic(const char *msg)
{
    laihost_log(999,msg);
    __builtin_unreachable();
}

void *laihost_malloc(size_t size)
{
    return malloc(size);
}

void *laihost_realloc(void *oldptr, size_t newsize, size_t oldsize)
{
    return realloc(oldptr,newsize);
}

void laihost_free(void *ptr, size_t size)
{
    return free(ptr);
}

void *laihost_map(size_t address, size_t count)
{
    for(int i = 0; i < count / VMM_PAGE + 1; i++)
        vmmMap(vmmGetBaseTable(),(void*)address,(void*)address,false,true);
    return NULL;
}

void laihost_unmap(void *pointer, size_t count)
{
    for(int i = 0; i < count / VMM_PAGE + 1; i++)
        vmmUnmap(vmmGetBaseTable(),pointer);
}

void *laihost_scan(const char *sig, size_t index)
{

}

void laihost_outb(uint16_t port, uint8_t val)
{
     iasm ("outb %%al,%%dx": :"d" (port), "a" (val));
}

void laihost_outw(uint16_t port, uint16_t val)
{
    iasm("outw %%ax, %%dx" : : "d"(port), "a"(val));
}

void laihost_outd(uint16_t port, uint32_t val)
{
    iasm("outl %%eax, %%dx" : : "d"(port), "a"(val));
}

uint8_t laihost_inb(uint16_t port)
{
    uint8_t ret;
    iasm ("inb %%dx,%%al":"=a" (ret):"d" (port));
    return ret;
}

uint16_t laihost_inw(uint16_t port)
{
    uint16_t ret;
    iasm ("inw %%dx,%%ax":"=a" (ret):"d" (port));
    return ret;
}

uint32_t laihost_ind(uint16_t port)
{
    uint32_t ret;
    iasm ("inl %%dx,%%eax":"=a" (ret):"d" (port));
    return ret;
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

struct acpi_rsdp *rsdp;

void acpiInit()
{
    // parse rsdp
    rsdp = (struct acpi_rsdp *)bootloaderGetRSDP()->rsdp;

    printk(" %p ",rsdp);
    printk("acpi %d ",rsdp->version);

    lai_set_acpi_revision(rsdp->version); // set acpi revision
}