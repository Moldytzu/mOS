#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <fw/bootloader.h>

struct psf1_header *font;
struct limine_file *fontMod;
struct limine_framebuffer *framebuffer;

struct framebuffer_cursor_info cursor; // info

// init the framebuffer
void framebufferInit()
{
    framebuffer = bootloaderGetFramebuffer(); // get the tag

    framebufferLoadFont("font-8x16.psf"); // load default font

    framebufferClear(0x000000); // clear framebuffer

    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner
}

// clear the framebuffer with a colour
inline void framebufferClear(uint32_t colour)
{
    cursor.X = cursor.Y = 0;                                                                                                                                                     // reset cursor position
    memset64((void *)framebuffer->address, (uint64_t)colour << 32 | colour, (framebuffer->pitch * framebuffer->height) / sizeof(uint64_t)); // clear the screen
}

// load a font
void framebufferLoadFont(const char *name)
{
    font = (struct psf1_header *)initrdGet(name);

    if (!font)
        goto error;

    if (font->magic[0] != PSF1_MAGIC0 && font->magic[0] != PSF1_MAGIC1) // if the psf1's magic isn't the one we expect, just fail
        goto error;

    return;

error: // show an error message
    bootloaderWrite("Failed to load font \"");
    bootloaderWrite(name);
    bootloaderWrite("\" from the initrd.\n");
    hang();
}

// plot pixel on the framebuffer
inline void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour)
{
    *(uint32_t *)((uint64_t)framebuffer->address + x * framebuffer->bpp / 8 + y * framebuffer->pitch) = colour; // set the pixel to colour
}

// plot character on the framebuffer
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

// create a new line
ifunc void newline()
{
    cursor.Y += font->charsize + 1; // add character's height and a 1 px padding
    cursor.X = 0; // reset cursor X

    if (cursor.Y + font->charsize + 1 >= framebuffer->height)
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

    if (cursor.X > framebuffer->width)
        newline();

    framebufferPlotc(c, cursor.X, cursor.Y);
    cursor.X += 8 + 1; // add character's width and a 1 px padding
}

// write a string
void framebufferWrite(const char *str)
{
    for (int i = 0; str[i]; i++)
        framebufferWritec(str[i]); // write characters
}

// get cursor information
struct framebuffer_cursor_info framebufferGetCursor()
{
    return cursor;
}

// overwrite cursor information
void framebufferSetCursor(struct framebuffer_cursor_info info)
{
    cursor = info;
}
