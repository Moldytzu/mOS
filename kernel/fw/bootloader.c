#include <fw/bootloader.h>

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0};

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

struct limine_terminal *terminal;

void bootloaderInit()
{
    if (terminal_request.response == NULL) // check for the response to be valid
        hang();

    terminal = terminal_request.response->terminals[0]; // point the default to the first terminal given by the bootloader
}

void bootloaderWrite(const char *str)
{
    terminal_request.response->write(terminal, str, strlen(str));
}

struct limine_file *bootloaderGetModule(const char *name)
{
    for (int i = 0; i < module_request.response->module_count; i++)
    {
        struct limine_file *m = module_request.response->modules[i];

        if(strcmp(m->path + 1 /*ignore the first slash*/, name) == 0) // compare the path names
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

void *bootloaderGetRSDP()
{
    return rsdp_request.response->address;
}

void *bootloaderGetHHDM()
{
    return (void *)hhdm_request.response->offset;
}