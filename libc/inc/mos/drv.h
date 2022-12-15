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
#define ISR(name) __attribute__((interrupt)) __attribute__((optimize("O0"))) __attribute__((__target__("no-sse"))) __attribute__((__target__("no-mmx"))) __attribute__((__target__("no-avx"))) void name(struct stack_frame *frame)

#define SYS_DRIVER_START 0
#define SYS_DRIVER_ANNOUNCE 1
#define SYS_DRIVER_FLUSH 2
#define SYS_DRIVER_IDT_SET 3
#define SYS_DRIVER_IDT_RESET 4
#define SYS_DRIVER_GET_PAGE_TABLE 5

#define SYS_DRIVER_TYPE_INPUT 1

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

typedef struct
{
    uint8_t keys[16];        // key buffers
    uint16_t mouseX, mouseY; // mouse coordonates
} drv_type_input_t;

uint64_t *sys_drv_announce(uint64_t type);
uint64_t sys_drv_start(char *path);
uint64_t sys_drv_page_table();
void sys_drv_set_page_table(uint64_t);
void sys_drv_flush(uint64_t type);
void sys_idt_set(void *handler, uint64_t vector);
void sys_idt_reset(uint64_t vector);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void picEOI();
void serialWritec(char c);