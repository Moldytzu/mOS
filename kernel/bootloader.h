#pragma once
#include <utils.h>
#include <stivale2.h>

void *bootloaderGetTag(struct stivale2_struct *stivale2_struct, uint64_t id);
void bootloaderInit(struct stivale2_struct *stivale2_struct);

void bootloaderTermWrite(const char *str);

struct stivale2_module bootloaderGetModule(const char *name);

struct stivale2_struct_tag_framebuffer *bootloaderGetFramebuf();
struct stivale2_struct_tag_memmap *bootloaderGetMemMap();
struct stivale2_struct_tag_kernel_base_address *bootloaderGetKernelAddr();

uint8_t bootloaderGetFirmwareType();