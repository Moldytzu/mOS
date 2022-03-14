#include <framebuffer.h>

struct stivale2_struct_tag_framebuffer *framebufTag;

void framebufferInit()
{
    framebufTag = bootloaderGetFramebuf();
    memset((void*)framebufTag->framebuffer_addr,0x00,framebufTag->framebuffer_pitch*framebufTag->framebuffer_height); // clear the screen with black
}