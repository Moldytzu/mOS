#include <framebuffer.h>

struct psf1_header *font;
struct stivale2_module fontMod;
struct stivale2_struct_tag_framebuffer *framebufTag;

struct framebuffer_cursor_info cursor; // info

void framebufferInit()
{
    framebufTag = bootloaderGetFramebuf(); // get the tag

    if (framebufTag->memory_model != 1) // check if we use RGB memory model
    {
        bootloaderTermWrite("Unsupported framebuffer memory model.\n");
        hang();
    }

    framebufferLoadFont("font-8x16.psf"); // load default font

    framebufferClear(0x000000); // clear framebuffer

    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0; // upper left corner
}

inline void framebufferClear(uint32_t colour)
{
    memset32((void *)framebufTag->framebuffer_addr, colour, framebufTag->framebuffer_pitch * framebufTag->framebuffer_height); // clear the screen
}

void framebufferLoadFont(const char *module)
{
    fontMod = bootloaderGetModule(module);                // get the module
    font = (struct psf1_header *)((void *)fontMod.begin); // cast the begining

    if (strlen(fontMod.string) != strlen(module)) // if the modules' string len doesn't match, just fail
        goto error;

    if (font->magic[0] != PSF1_MAGIC0 && font->magic[0] != PSF1_MAGIC1) // if the psf1's magic isn't the one we expect, just fail
        goto error;

    return;

error: // show an error message
    bootloaderTermWrite("Failed to load font \"");
    bootloaderTermWrite(module);
    bootloaderTermWrite("\".\n");
    hang();
}

inline void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour)
{
    *(uint32_t *)((uint64_t)framebufTag->framebuffer_addr + x * framebufTag->framebuffer_bpp / 8 + y * framebufTag->framebuffer_pitch) = colour; // set the pixel to colour
}

void framebufferPlotc(char c, uint32_t x, uint32_t y)
{
    uint8_t *character = (uint8_t *)font + sizeof(struct psf1_header) + c * font->charsize; // get the offset by skipping the header and indexing the character
    for (size_t dy = 0; dy < font->charsize; dy++, character++)                             // loop thru each line of the character
    {
        for (size_t dx = 0; dx < 8; dx++) // 8 pixels wide
        {
            uint8_t mask = 0b10000000 >> dx; // create a bitmask that shifts based on which pixel from the line we're indexing
            if (*character & mask)           // and the mask with the line
                framebufferPlotp(dx + x, dy + y, cursor.colour);
        }
    }
}

void framebufferWrite(const char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '\n' || cursor.X > framebufTag->framebuffer_width) // new line or the cursor isn't in view
        {
            cursor.Y += font->charsize + 1; // add character's height and a 1 px padding
            cursor.X = 0;                   // reset cursor X
            continue;
        }

        framebufferPlotc(str[i], cursor.X, cursor.Y);
        cursor.X += 8 + 1; // add character's width and a 1 px padding
    }
}

struct framebuffer_cursor_info framebufferGetCursor()
{
    return cursor;
}

void framebufferSetCursor(struct framebuffer_cursor_info info)
{
    cursor = info;
}