#pragma once
#include <mos/sys.h>

// utilites and defintitions for easier driver development

struct stack_frame;

#define pack __attribute__((__packed__))
#define toMB(x) ((x) / 1024 / 1024)
#define toKB(x) ((x) / 1024)
#define align(val, alg) (max((uint64_t)(val), alg) + (alg - (max((uint64_t)(val), alg) % alg)))
#define alignD(val, alg) (align(val, alg) - alg)
#define unsafe_cast(val, type) (*(type *)&val)
#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define between(a, b, c) (((uint64_t)(a) >= (uint64_t)(b)) && ((uint64_t)(a) <= (uint64_t)(c)))
#define pstruct typedef struct __attribute__((__packed__))

#define SYS_DRIVER_START 0
#define SYS_DRIVER_ANNOUNCE 1
#define SYS_DRIVER_FLUSH 2
#define SYS_DRIVER_IDT_SET 3
#define SYS_DRIVER_IDT_RESET 4
#define SYS_DRIVER_GET_PCI_DEVICE 5
#define SYS_DRIVER_IDENTITY_MAP 6

#define SYS_DRIVER_TYPE_INPUT 1
#define SYS_DRIVER_TYPE_FRAMEBUFFER 2

#define PIC_EOI_CMD 0x20
#define PIC_INIT_CMD 0x11

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DAT 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DAT 0xA1

#define PIC_IRQ_BASE 0x20
#define PIC_IRQ_0 (PIC_IRQ_BASE + 0)
#define PIC_IRQ_1 (PIC_IRQ_BASE + 1)
#define PIC_IRQ_2 (PIC_IRQ_BASE + 2)
#define PIC_IRQ_3 (PIC_IRQ_BASE + 3)
#define PIC_IRQ_4 (PIC_IRQ_BASE + 4)
#define PIC_IRQ_5 (PIC_IRQ_BASE + 5)
#define PIC_IRQ_6 (PIC_IRQ_BASE + 6)
#define PIC_IRQ_7 (PIC_IRQ_BASE + 7)
#define PIC_IRQ_8 (PIC_IRQ_BASE + 8)
#define PIC_IRQ_9 (PIC_IRQ_BASE + 9)
#define PIC_IRQ_10 (PIC_IRQ_BASE + 10)
#define PIC_IRQ_11 (PIC_IRQ_BASE + 11)
#define PIC_IRQ_12 (PIC_IRQ_BASE + 12)
#define PIC_IRQ_13 (PIC_IRQ_BASE + 13)

pstruct
{
    uint8_t keys[16];        // key buffers
    uint16_t mouseX, mouseY; // mouse coordonates
}
drv_type_input_t;

pstruct
{
    void *base; // base address
    uint32_t currentXres, currentYres;
    uint32_t requestedXres, requestedYres;
}
drv_type_framebuffer_t;

pstruct
{
    uint16_t vendor;
    uint16_t device;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t programInterface;
    uint8_t subclass;
    uint8_t class;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
}
drv_pci_header_t;

pstruct
{
    uint16_t VendorID;
    uint16_t DeviceID;
    uint16_t Command;
    uint16_t Status;
    uint8_t RevisionID;
    uint8_t ProgramInterface;
    uint8_t Subclass;
    uint8_t Class;
    uint8_t CacheLineSize;
    uint8_t LatencyTimer;
    uint8_t HeaderType;
    uint8_t BIST;
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t CardBusCISPtr;
    uint16_t SubsystemVendorID;
    uint16_t SubsystemID;
    uint16_t ExpansionRomBaseAddr;
    uint16_t CapabilitiesPtr;
    uint16_t Rsv0;
    uint16_t Rsv1;
    uint16_t Rsv2;
    uint8_t IntreruptLine;
    uint8_t IntreruptPin;
    uint8_t MinGrant;
    uint8_t MaxLatency;
}
drv_pci_header0_t;

uint64_t *sys_drv_announce(uint64_t type);
uint64_t sys_drv_start(char *path);
drv_pci_header_t *sys_pci_get(uint32_t vendor, uint32_t device);
void sys_drv_flush(uint64_t type);
void sys_idt_set(void *handler, uint64_t vector);
void sys_idt_reset(uint64_t vector);
void sys_identity_map(void *address);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void picEOI();
void serialWritec(char c);