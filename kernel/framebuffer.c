#include <framebuffer.h>

struct stivale2_struct_tag_framebuffer *framebufTag;

void framebufferInit()
{
    framebufTag = bootloaderGetFramebuf();

    if(framebufTag->memory_model != 1)
    {
        bootloaderTermWrite("Unsupported framebuffer memory model.\n");
        hang();
    }

    framebufferClear(0x000000);
}

void framebufferClear(uint32_t colour)
{
    memset32((void*)framebufTag->framebuffer_addr,colour,framebufTag->framebuffer_pitch*framebufTag->framebuffer_height); // clear the screen
}