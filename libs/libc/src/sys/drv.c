#include <mos/drv.h>

void outb(uint16_t port, uint8_t val) // out byte
{
    iasm("outb %0, %1" ::"a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) // in byte
{
    uint8_t val;
    iasm("inb %%dx,%%al"
         : "=a"(val)
         : "d"(port));
    return val;
}

void outw(uint16_t port, uint16_t val) // out word
{
    iasm("outw %0, %1" ::"a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) // in word
{
    uint16_t val;
    iasm("inw %%dx,%%ax"
         : "=a"(val)
         : "d"(port));
    return val;
}

void outl(uint16_t port, uint32_t val)
{
    iasm("outl %0, %1" ::"a"(val), "d"(port));
}

uint32_t inl(uint16_t port)
{
    uint32_t val;
    iasm("inl %1,%0"
         : "=a"(val)
         : "d"(port));
    return val;
}

uint64_t *sys_driver_announce(uint64_t type)
{
    return (uint64_t *)sys_driver(SYS_DRIVER_ANNOUNCE, type, 0, 0);
}

uint64_t sys_driver_flush(uint64_t type)
{
    return sys_driver(SYS_DRIVER_FLUSH, type, 0, 0); // flush the context for that type
}

uint64_t sys_idt_set(void *handler, uint64_t vector)
{
    return sys_driver(SYS_DRIVER_IDT_SET, (uint64_t)handler, vector, 0);
}

uint64_t sys_idt_reset(uint64_t vector)
{
    return sys_driver(SYS_DRIVER_IDT_RESET, vector, 0, 0);
}

uint64_t sys_driver_start(char *path)
{
    return sys_driver(SYS_DRIVER_START, (uint64_t)path, 0, 0);
}

drv_pci_header_t *sys_pci_get(uint32_t vendor, uint32_t device)
{
    return (drv_pci_header_t *)sys_driver(SYS_DRIVER_GET_PCI_DEVICE, vendor, device, 0);
}

uint64_t sys_identity_map(void *address)
{
    return sys_driver(SYS_DRIVER_IDENTITY_MAP, (uint64_t)address, 0, 0);
}

void serialWritec(char c)
{
    outb(0x3F8, c); // output the character on the serial console
}

uint8_t sys_driver_allocate_vector()
{
    return sys_driver(SYS_DRIVER_ALLOCATE_VECTOR, 0, 0, 0);
}

uint64_t sys_driver_deallocate_vector(uint8_t vector)
{
    return sys_driver(SYS_DRIVER_DEALLOCATE_VECTOR, vector, 0, 0);
}