#pragma once
#include <misc/utils.h>
#include <limine.h>

struct limine_file *bootloaderGetModule(const char *name);
struct limine_framebuffer *bootloaderGetFramebuffer();
struct limine_memmap_response *bootloaderGetMemoryMap();
struct limine_kernel_address_response *bootloaderGetKernelAddress();
struct limine_smp_response *bootloaderGetSMP();
void *bootloaderGetRSDP();
void *bootloaderGetHHDM();
char *bootloaderGetCommandLine();