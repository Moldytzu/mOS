#include <framebuffer.h>

struct psf1_header *font;
struct stivale2_module fontMod;
struct stivale2_struct_tag_framebuffer *framebufTag;

void framebufferInit()
{
    framebufTag = bootloaderGetFramebuf();

    if (framebufTag->memory_model != 1)
    {
        bootloaderTermWrite("Unsupported framebuffer memory model.\n");
        hang();
    }

    framebufferLoadFont("font-8x16.psf"); // load default font

    framebufferClear(0x000000); // clear framebuffer
}

void framebufferClear(uint32_t colour)
{
    memset32((void *)framebufTag->framebuffer_addr, colour, framebufTag->framebuffer_pitch * framebufTag->framebuffer_height); // clear the screen
}

void framebufferLoadFont(const char *module)
{
    fontMod = bootloaderGetModule(module);
    font = (struct psf1_header *)((void *)fontMod.begin);

    if(strlen(fontMod.string) != strlen(module)) // if the modules' string len doesn't match, just fail
        goto error;

    if (font->magic[0] != PSF1_MAGIC0 && font->magic[0] != PSF1_MAGIC1) // if the psf1's magic isn't the one we expect, just fail
        goto error;

    return;

error:
    bootloaderTermWrite("Failed to load font \"");
    bootloaderTermWrite(module);
    bootloaderTermWrite("\".\n");
    hang();
}

void framebufferPlotc(char c, uint32_t x, uint32_t y)
{
    uint32_t offset = sizeof(struct psf1_header) + c * 16; // get the offset by skipping the header and indexing the character
    for(size_t dy = 0; dy < font->charsize; dy++) // loop thru each line of the character
    {
        for(size_t dx = 0; dx < 8; dx++) // 8 pixels wide
        {
            if((*(uint8_t*)(font + offset + dy) & (0b10000000 >> dx)) > 0)
                *(uint32_t*)((uint32_t*)framebufTag->framebuffer_addr + dy + (dx * framebufTag->framebuffer_width)) = 0xFFFFFF;
        }
    }
}