#include <mos/drv.h>

void outb(uint16_t port, uint8_t val) // out byte
{
    asm volatile("outb %0, %1" ::"a"(val), "Nd"(port));
}

uint64_t *sys_drv_announce(uint64_t type)
{
    uint64_t address;
    sys_driver(SYS_DRIVER_ANNOUNCE, type, (uint64_t)&address); // announce that we want the struct for the requested type
    return (uint64_t *)address;                                // return the pointer to the struct
}

void sys_drv_flush(uint64_t type)
{
    sys_driver(SYS_DRIVER_FLUSH, type, 0); // flush the context for that type
}