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

    framebufferLoadFont("font-8x16.psf"); // load default font

    framebufferClear(0x000000);
}

void framebufferClear(uint32_t colour)
{
    memset32((void*)framebufTag->framebuffer_addr,colour,framebufTag->framebuffer_pitch*framebufTag->framebuffer_height); // clear the screen
}

void framebufferLoadFont(const char *module)
{
    font = bootloaderGetModule(module);
    if(strlen(font.string) != strlen(module))
    {
        bootloaderTermWrite("Failed to load font \"");
        bootloaderTermWrite(module);
        bootloaderTermWrite("\".\n");
        hang();
    }
}