#include <framebuffer.h>

struct stivale2_module font;
struct stivale2_struct_tag_framebuffer *framebufTag;

void framebufferInit()
{
    framebufTag = bootloaderGetFramebuf();

    if(framebufTag->memory_model != 1)
    {
        bootloaderTermWrite("Unsupported framebuffer memory model.\n");
        hang();
    }

    font = bootloaderGetModule("font-8x16.psf"); // default font
    if(strlen(font.string) != strlen("font-8x16.psf")) // check the strlen
    {
        bootloaderTermWrite("Failed to load default font \"font-8x16.psf\".\n");
        hang();
    }

    framebufferClear(0x000000);
}

void framebufferClear(uint32_t colour)
{
    memset32((void*)framebufTag->framebuffer_addr,colour,framebufTag->framebuffer_pitch*framebufTag->framebuffer_height); // clear the screen
}