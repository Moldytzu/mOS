#pragma once
#include <mos/sys.h>

// utilites and defintitions for easier driver development

typedef struct
{
    uint8_t keys[16];        // key buffers
    uint16_t mouseX, mouseY; // mouse coordonates
} drv_type_input_t;

uint64_t *sys_drv_announce(uint64_t type);
void sys_drv_flush(uint64_t type);
void outb(uint16_t port, uint8_t val);