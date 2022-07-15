#pragma once
#include <misc/utils.h>
#include <stivale2.h>

void *bootloaderGetTag(uint64_t id);
void bootloaderInit(struct stivale2_struct *stivale2_struct);

void bootloaderTermWrite(const char *str);

struct stivale2_module bootloaderGetModule(const char *name);
bool bootloaderProbePML5();

struct stivale2_struct_tag_framebuffer *bootloaderGetFramebuf();
struct stivale2_struct_tag_memmap *bootloaderGetMemMap();
struct stivale2_struct_tag_kernel_base_address *bootloaderGetKernelAddr();
struct stivale2_struct_tag_pmrs *bootloaderGetPMRS();
struct stivale2_struct_tag_rsdp *bootloaderGetRSDP();
void *bootloaderGetHHDM();

uint8_t bootloaderGetFirmwareType();