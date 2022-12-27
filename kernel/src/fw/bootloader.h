#pragma once
#include <misc/utils.h>
#include <limine.h>

void bootloaderInit();

void bootloaderWrite(const char *str);

struct limine_file *bootloaderGetModule(const char *name);
struct limine_framebuffer *bootloaderGetFramebuffer();
struct limine_memmap_response *bootloaderGetMemoryMap();
struct limine_kernel_address_response *bootloaderGetKernelAddress();
void *bootloaderGetRSDP();
void *bootloaderGetHHDM();