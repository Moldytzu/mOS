#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>

psf1_header_t *font;
struct limine_file *fontMod;
struct limine_framebuffer framebuffer;

framebuffer_cursor_info_t cursor;              // info
drv_type_framebuffer_t drv_type_framebuffer_s; // driver structure

// init the framebuffer
void framebufferInit()
{
    memcpy(&framebuffer, bootloaderGetFramebuffer(), sizeof(struct limine_framebuffer)); // get the tag

    framebufferLoadFont("font-8x16.psf"); // load default font

    framebufferClear(0x000000); // clear framebuffer

    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    // set in the context
    drv_type_framebuffer_s.base = framebuffer.address;
    drv_type_framebuffer_s.currentXres = drv_type_framebuffer_s.requestedXres = framebuffer.width;
    drv_type_framebuffer_s.currentYres = drv_type_framebuffer_s.requestedYres = framebuffer.height;

    printk("fb: display resolution is %dx%d\n", framebuffer.width, framebuffer.height);
}

void framebufferFlush()
{
    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    framebuffer.address = drv_type_framebuffer_s.base;
    framebuffer.width = drv_type_framebuffer_s.currentXres;
    framebuffer.height = drv_type_framebuffer_s.currentYres;
    framebuffer.pitch = drv_type_framebuffer_s.currentXres * 4;

    // map the framebuffer
    for (int i = 0; i < framebuffer.pitch * framebuffer.height; i += 4096)
        vmmMap(vmmGetBaseTable(), framebuffer.address + i, framebuffer.address + i, false, true);
}

// clear the framebuffer with a colour
inline void framebufferClear(uint32_t colour)
{
    cursor.X = cursor.Y = 0; // reset cursor position

    if (!colour) // if the colour is null then we want everything to be zeroed out
        zero(framebuffer.address, framebuffer.pitch * framebuffer.height);
    else
        memset(framebuffer.address, colour, framebuffer.pitch * framebuffer.height); // todo: optimise this even though we don't clear with colour
}

// load a font
void framebufferLoadFont(const char *name)
{
    font = (psf1_header_t *)initrdGet(name);

    if (!font)
        goto error;

    if (font->magic[0] != PSF1_MAGIC0 && font->magic[0] != PSF1_MAGIC1) // if the psf1's magic isn't the one we expect, just fail
        goto error;

    return;

error: // show an error message
    bootloaderWrite("fb: failed to load font \"");
    bootloaderWrite(name);
    bootloaderWrite("\" from the initrd.\n");
    hang();
}

// plot pixel on the framebuffer
inline void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour)
{
    *(uint32_t *)((uint64_t)framebuffer.address + x * framebuffer.bpp / 8 + y * framebuffer.pitch) = colour; // set the pixel to colour
}

// plot character on the framebuffer
void framebufferPlotc(char c, uint32_t x, uint32_t y)
{
    uint8_t *character = (uint8_t *)font + sizeof(psf1_header_t) + c * font->charsize; // get the offset by skipping the header and indexing the character
    for (size_t dy = 0; dy < font->charsize; dy++, character++)                        // loop thru each line of the character
    {
        for (size_t dx = 0; dx < 8; dx++) // 8 pixels wide
        {
            uint8_t mask = 0b10000000 >> dx; // create a bitmask that shifts based on which pixel from the line we're indexing
            if (*character & mask)           // and the mask with the line
                framebufferPlotp(dx + x, dy + y, cursor.colour);
        }
    }
}

// create a new line
ifunc void newline()
{
    cursor.Y += font->charsize + 1; // add character's height and a 1 px padding
    cursor.X = 0;                   // reset cursor X

    if (cursor.Y + font->charsize + 1 >= framebuffer.height)
    {
        cursor.Y = 0;
        framebufferClear(0);
    }
}

// write a character
void framebufferWritec(char c)
{
    if (c == '\n') // new line
    {
        newline();
        return;
    }

    if (cursor.X + 8 > framebuffer.width)
        newline();

    framebufferPlotc(c, cursor.X, cursor.Y);
    cursor.X += 8 + 1; // add character's width and a 1 px padding
}

// write a string
void framebufferWrite(const char *str)
{
    if (!str)
        return;

    for (int i = 0; str[i]; i++)
        framebufferWritec(str[i]); // write characters
}

// get cursor information
framebuffer_cursor_info_t framebufferGetCursor()
{
    return cursor;
}

// overwrite cursor information
void framebufferSetCursor(framebuffer_cursor_info_t info)
{
    cursor = info;
}
