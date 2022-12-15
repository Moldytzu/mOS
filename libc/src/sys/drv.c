#include <mos/drv.h>

void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
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

uint64_t *sys_drv_announce(uint64_t type)
{
    uint64_t address = 0;
    sys_driver(SYS_DRIVER_ANNOUNCE, type, (uint64_t)&address, 0); // announce that we want the struct for the requested type
    return (uint64_t *)address;                                   // return the pointer to the struct
}

void sys_drv_flush(uint64_t type)
{
    sys_driver(SYS_DRIVER_FLUSH, type, 0, 0); // flush the context for that type
}

void sys_idt_set(void *handler, uint64_t vector)
{
    sys_driver(SYS_DRIVER_IDT_SET, (uint64_t)handler, vector, 0);
}

void sys_idt_reset(uint64_t vector)
{
    sys_driver(SYS_DRIVER_IDT_RESET, vector, 0, 0);
}

uint64_t sys_drv_start(char *path)
{
    uint64_t pid = 0;
    sys_driver(SYS_DRIVER_START, (uint64_t)path, (uint64_t)&pid, 0);
    return pid;
}

drv_pci_header_t *sys_pci_get(uint32_t vendor, uint32_t device)
{
    uint64_t header = 0;
    sys_driver(SYS_DRIVER_GET_PCI_DEVICE, (uint64_t)&header, vendor, device);
    return (drv_pci_header_t *)header;
}

void sys_drv_set_page_table(uint64_t table)
{
    iasm("mov %0, %%cr3" ::"r"(table));
}

void picEOI()
{
    outb(PIC_SLAVE_CMD, PIC_EOI_CMD);
    outb(PIC_MASTER_CMD, PIC_EOI_CMD);
}

void serialWritec(char c)
{
    outb(0x3F8, c); // output the character on the serial console
}
