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
#define SYS_DRIVER_REDIRECT_IRQ_TO_VECTOR 7
#define SYS_DRIVER_ALLOCATE_VECTOR 8
#define SYS_DRIVER_DEALLOCATE_VECTOR 9

#define SYS_DRIVER_TYPE_FRAMEBUFFER 1
#define SYS_DRIVER_TYPE_INPUT 2

pstruct
{
    uint32_t pid;
    uint8_t keys[64];        // key buffers
    int mouseX, mouseY;      // mouse relative coordonates
    uint8_t mouseButtons[5]; // mouse button states
}
drv_type_input_t;

#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_MIDDLE 1
#define MOUSE_BUTTON_RIGHT 2

pstruct
{
    uint32_t pid;
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

uint64_t *sys_driver_announce(uint64_t type);
uint64_t sys_driver_start(char *path);
drv_pci_header_t *sys_pci_get(uint32_t vendor, uint32_t device);
uint64_t sys_driver_flush(uint64_t type);
uint64_t sys_idt_set(void *handler, uint64_t vector);
uint64_t sys_idt_reset(uint64_t vector);
uint64_t sys_identity_map(void *address);
uint8_t sys_driver_allocate_vector();
uint64_t sys_driver_deallocate_vector(uint8_t vector);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t val);
uint16_t inw(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void serialWritec(char c);