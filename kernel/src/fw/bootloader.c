#include <fw/bootloader.h>
#include <mm/pmm.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0};

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0};

static volatile struct limine_stack_size_request stack_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 4 * 1024};

struct limine_file *bootloaderGetModule(const char *name)
{
    for (int i = 0; i < module_request.response->module_count; i++)
    {
        struct limine_file *m = module_request.response->modules[i];

        if (strcmp(m->path + 1 /*ignore the first slash*/, name) == 0) // compare the path names
            return m;
    }

    return NULL;
}

struct limine_framebuffer *bootloaderGetFramebuffer()
{
    return framebuffer_request.response->framebuffers[0];
}

struct limine_memmap_response *bootloaderGetMemoryMap()
{
    return memmap_request.response;
}

struct limine_kernel_address_response *bootloaderGetKernelAddress()
{
    return kernel_address_request.response;
}

struct limine_smp_response *bootloaderGetSMP()
{
    return smp_request.response;
}

void *bootloaderGetRSDP()
{
    return rsdp_request.response->address;
}

void *bootloaderGetHHDM()
{
    return (void *)hhdm_request.response->offset;
}