#include <fw/bootloader.h>

struct stivale2_struct_tag_kernel_base_address *baseAddrTag;
struct stivale2_struct_tag_framebuffer *framebufTag;
struct stivale2_struct_tag_terminal *termTag;
struct stivale2_struct_tag_modules *modsTag;
struct stivale2_struct_tag_memmap *memTag;
struct stivale2_struct_tag_firmware *fwTag;
struct stivale2_struct_tag_pmrs *pmrTag;
struct stivale2_struct_tag_hhdm *hhdmTag;
struct stivale2_struct_tag_rsdp *rsdpTag;
struct stivale2_struct *stivale2struct;

void (*termWrite)(const char *string, size_t length);

// We need to tell the stivale bootloader where we want our stack to be.
static uint8_t stack[8 * 1024]; // 8 kb stack

// terminal
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID, // id
        .next = 0                                      // end
    },
    .flags = 0 // unused
};

// framebuffer
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&terminal_hdr_tag},
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};

// stivale header
__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    .entry_point = 0,                          // elf entry point
    .stack = (uintptr_t)stack + sizeof(stack), // stack
    .flags = 0b1111,
    .tags = (uintptr_t)&framebuffer_hdr_tag // root of the linked list
};

// get tag
void *bootloaderGetTag(uint64_t id)
{
    struct stivale2_tag *current_tag = (void *)stivale2struct->tags;
    while (true)
    {
        // check if the list is over
        if (current_tag == NULL)
            return NULL;

        // check whether the identifier matches.
        if (current_tag->identifier == id)
            return current_tag;

        // get the next tag
        current_tag = (void *)current_tag->next;
    }
}

// init stivale2 bootloader
void bootloaderInit(struct stivale2_struct *stivale2_struct)
{
    stivale2struct = stivale2_struct;
    termTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_TERMINAL_ID); // get terminal
    termWrite = (void *)termTag->term_write;                                      // set write function

    framebufTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);         // get frame buffer info
    modsTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_MODULES_ID);                 // get modules info
    memTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_MEMMAP_ID);                   // get memory map
    fwTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_FIRMWARE_ID);                  // get firmware information
    baseAddrTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID); // get kernel base address
    pmrTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_PMRS_ID);                     // get protected memory ranges
    hhdmTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_HHDM_ID);                    // get higher half direct mapping base
    rsdpTag = bootloaderGetTag(STIVALE2_STRUCT_TAG_RSDP_ID);
}

// write to stivale2 terminal
void bootloaderTermWrite(const char *str)
{
    termWrite(str, strlen(str));
}

// get stivale2 frame buffer
struct stivale2_struct_tag_framebuffer *bootloaderGetFramebuf()
{
    return framebufTag;
}

// get a specific module
struct stivale2_module bootloaderGetModule(const char *name)
{
    for (uint64_t i = 0; i < modsTag->module_count; i++)
    {
        if (strlen(modsTag->modules[i].string) != strlen(name))
            continue;                                                                    // if the lenghts differ we don't have to check the name byte by byte
        if (memcmp8((void *)modsTag->modules[i].string, (void *)name, strlen(name)) == 0) // if the check is successful we return the module
            return modsTag->modules[i];
    }

    return *(struct stivale2_module *)NULL; // return a null pointer
}

// get memory map
struct stivale2_struct_tag_memmap *bootloaderGetMemMap()
{
    return memTag;
}

// get firmware type
uint8_t bootloaderGetFirmwareType()
{
    return fwTag->flags & 0b1; // 1 bios, 0 uefi
}

// get kernel address
struct stivale2_struct_tag_kernel_base_address *bootloaderGetKernelAddr()
{
    return baseAddrTag;
}

// get protected memory ranges
struct stivale2_struct_tag_pmrs *bootloaderGetPMRS()
{
    return pmrTag;
}

// get hhdm
void *bootloaderGetHHDM()
{
    return (void *)hhdmTag->addr;
}

// get rsdp
struct stivale2_struct_tag_rsdp *bootloaderGetRSDP()
{
    return rsdpTag;
}

// probe for pml5 support
bool bootloaderProbePML5()
{
    if(bootloaderGetTag(STIVALE2_HEADER_TAG_5LV_PAGING_ID))
    {
        #ifdef K_BLDR_DEBUG
            printks("bldr: level 5 paging supported\n\r");
        #endif
        return true;
    }

    return false;
}