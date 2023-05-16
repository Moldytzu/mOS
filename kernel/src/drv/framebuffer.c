#include <drv/framebuffer.h>
#include <drv/initrd.h>
#include <drv/drv.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <misc/logger.h>
#include <sched/hpet.h>

extern uint8_t _binary____kfont_psf_start; // the font file embeded in the kernel as a symbol

psf2_header_t *font;
struct limine_framebuffer framebuffer;

framebuffer_cursor_info_t cursor; // info

void framebufferPlotc(char c, uint32_t x, uint32_t y);

#define FB_BENCHMARK_SIZE 1000000
void framebufferBenchmark()
{
    // test performance of the mapper
    logInfo("fb: benchmarking");

    uint64_t start = hpetMillis();

    for (int i = 0; i < FB_BENCHMARK_SIZE; i++)
        framebufferPlotc('A', 0, 0);

    uint64_t end = hpetMillis();

    logInfo("fb: it took %d miliseconds (%d chars/mili)", end - start, FB_BENCHMARK_SIZE / (end - start));

    hang();
}

bool checkFont(psf2_header_t *font)
{
    if (!font)
        return false;

    if (font->magic[0] != PSF2_MAGIC0 || font->magic[1] != PSF2_MAGIC1 || font->magic[2] != PSF2_MAGIC2 || font->magic[3] != PSF2_MAGIC3)
        return false;

    return true;
}

// init the framebuffer
void framebufferInit()
{
    memcpy(&framebuffer, bootloaderGetFramebuffer(), sizeof(struct limine_framebuffer)); // get the tag

    font = (psf2_header_t *)&_binary____kfont_psf_start; // use embeded kernel font
    if (!checkFont(font))                                // trigger a red screen of death
    {
        memset32(framebuffer.address, 0xFF0000, framebuffer.width * framebuffer.pitch / sizeof(uint32_t));
        hang();
    }

    framebufferZero(); // clear framebuffer

    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    logInfo("fb: display resolution is %dx%d", framebuffer.width, framebuffer.height);
}

void framebufferFlush()
{
    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    drv_context_fb_t *ctx = (drv_context_fb_t *)drvQueryActive(DRV_TYPE_FB);

    if (!ctx || !ctx->base)
        return;

    framebuffer.address = ctx->base;
    framebuffer.width = ctx->currentXres;
    framebuffer.height = ctx->currentYres;
    framebuffer.pitch = ctx->currentXres * 4;

    // map the framebuffer
    for (int i = 0; i < framebuffer.pitch * framebuffer.height; i += 4096)
        vmmMap(vmmGetBaseTable(), framebuffer.address + i, framebuffer.address + i, VMM_ENTRY_RW | VMM_ENTRY_WRITE_THROUGH);
}

// clear the framebuffer with a colour
inline void framebufferClear(uint32_t colour)
{
    cursor.X = cursor.Y = 0; // reset cursor position

    memset32(framebuffer.address, colour, framebuffer.pitch * framebuffer.height); // todo: optimise this even though we don't clear with colour
}

// zeros the whole framebuffer
void framebufferZero()
{
    cursor.X = cursor.Y = 0;                                           // reset cursor position
    zero(framebuffer.address, framebuffer.pitch * framebuffer.height); // clear the framebuffer
}

// plot pixel on the framebuffer
ifunc void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour)
{
    *(uint32_t *)((uint64_t)framebuffer.address + x * framebuffer.bpp / 8 + y * framebuffer.pitch) = colour; // set the pixel to colour
}

// plot character on the framebuffer
void framebufferPlotc(char c, uint32_t x, uint32_t y)
{
    uint16_t pitch = font->charsize / font->height;
    uint8_t *character = (uint8_t *)font + font->headersize + c * font->charsize; // get the offset by skipping the header and indexing the character

    for (size_t dy = 0; dy < font->height; dy++) // loop for each pixel of character
        for (size_t dx = 0; dx < font->width; dx++)
            if ((character[dy * pitch + dx / 8] >> (7 - dx % 8)) & 1) // create a bit mask then and in the current byte of the character
                framebufferPlotp(dx + x, dy + y, cursor.colour);
}

// create a new line
ifunc void newline()
{
    cursor.Y += font->height + 1; // add character's height and a 1 px padding
    cursor.X = 0;                 // reset cursor X

    if (cursor.Y + font->height + 1 >= framebuffer.height) // we can't write further
    {
#ifdef K_FB_SCROLL
        // todo: optimise this by not reading video memory
        cursor.Y -= font->height + 1; // move back to the last line

        size_t fbSize = framebuffer.pitch * framebuffer.height; // size in bytes of the frame buffer
        size_t offset = font->height * framebuffer.pitch;       // get offset of the second line
        size_t bytes = fbSize - offset;                         // calculate the bytes to be copied

        memcpy64(framebuffer.address, framebuffer.address + offset, bytes / 8); // do the actual copy

        memset64(framebuffer.address + bytes, 0, (fbSize - bytes) / 8); // zero last line
#else
        cursor.Y = 0;
        framebufferZero();
#endif
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

    if (cursor.X + font->width > framebuffer.width)
        newline();

    if (c == ' ' && cursor.X == 0)
        return;

    framebufferPlotc(c, cursor.X, cursor.Y);
    cursor.X += font->width + 1; // add character's width and a 1 px padding
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
