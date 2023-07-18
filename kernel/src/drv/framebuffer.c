#include <drv/framebuffer.h>
#include <drv/drv.h>
#include <fs/initrd.h>
#include <fw/bootloader.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <misc/logger.h>
#include <sched/hpet.h>

extern uint8_t _binary____kfont_psf_start; // the font file embeded in the kernel as a symbol

psf2_header_t *font;
uint16_t fontPitch;
struct limine_framebuffer framebuffer;

#ifdef K_FB_DOUBLE_BUFFER
struct limine_framebuffer back;
locker_t fbLock;
#else
#define back framebuffer
#undef lock
#define lock(x, y) \
    {              \
        y;         \
    }
#endif

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
    memcpy(&back, bootloaderGetFramebuffer(), sizeof(struct limine_framebuffer));        // copy metadata to the back buffer

    font = (psf2_header_t *)&_binary____kfont_psf_start; // use embeded kernel font
    if (!checkFont(font))
    {
        framebufferClear(0xFF0000); // trigger a red screen of death
        hang();
    }

    fontPitch = font->charsize / font->height; // precalculate pitch

    framebufferZero(); // clear framebuffer

    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    logInfo("fb: display resolution is %dx%d at %p", back.width, back.height, back.address);
}

void framebufferFlush()
{
    cursor.colour = 0xFFFFFF; // white cursor
    cursor.X = cursor.Y = 0;  // upper left corner

    drv_context_fb_t *ctx = (drv_context_fb_t *)drvQueryActive(DRV_TYPE_FB);

    if (!ctx || !ctx->base)
        return;

    lock(fbLock, {
        if (back.address != framebuffer.address) // deallocate
            pmmDeallocatePages(back.address, (back.pitch * back.height) / PMM_PAGE + 1);

        framebuffer.address = ctx->base;
        framebuffer.width = ctx->currentXres;
        framebuffer.height = ctx->currentYres;
        framebuffer.pitch = ctx->currentXres * 4;

        memcpy(&back, &framebuffer, sizeof(struct limine_framebuffer)); // sync metadata

        // map the framebuffer
        for (int i = 0; i < framebuffer.pitch * framebuffer.height; i += 4096)
            vmmMapKernel(framebuffer.address + i, framebuffer.address + i, VMM_ENTRY_RW | VMM_ENTRY_WRITE_THROUGH);
    });

    framebufferInitDoubleBuffer(); // quickly generate a new buffer
}

// clear the framebuffer with a colour
inline void framebufferClear(uint32_t colour)
{
    lock(fbLock, {
        cursor.X = cursor.Y = 0; // reset cursor position

        memset32(back.address, colour, (back.pitch * back.height) / sizeof(uint32_t)); // todo: optimise this even though we don't clear with colour
    });
}

// inits the back buffer
void framebufferInitDoubleBuffer()
{
#ifdef K_FB_DOUBLE_BUFFER
    size_t fbPages = (back.pitch * back.height) / PMM_PAGE + 1;
    lock(fbLock, {
        back.address = pmmPages(fbPages);
    });
    logInfo("allocated %d pages for back buffer", fbPages);
#endif
}

// copies back buffer to the front buffer
void framebufferUpdate()
{
    lock(fbLock, {
#ifdef K_FB_DOUBLE_BUFFER
        if (framebuffer.address != back.address) // don't copy the same buffer to itself
            memcpy64(framebuffer.address, back.address, (back.pitch * back.height) / sizeof(uint64_t));
#endif
    });
}

// zeros the whole framebuffer
void framebufferZero()
{
    lock(fbLock, {
        cursor.X = cursor.Y = 0;                      // reset cursor position
        zero(back.address, back.pitch * back.height); // clear the framebuffer
    });
}

// plot pixel on the framebuffer
ifunc void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour)
{
    *(uint32_t *)((uint64_t)back.address + x * back.bpp / 8 + y * back.pitch) = colour; // set the pixel to colour
}

// function to inline
ifunc void framebufferPlotc_impl(char c, uint32_t x, uint32_t y)
{
    uint8_t *character = (uint8_t *)font + font->headersize + c * font->charsize; // get the offset by skipping the header and indexing the character

    for (size_t dy = 0; dy < font->height; dy++) // loop for each pixel of character
        for (size_t dx = 0; dx < font->width; dx++)
            if ((character[dy * fontPitch + dx / 8] >> (7 - dx % 8)) & 1) // create a bit mask then and in the current byte of the character
                framebufferPlotp(dx + x, dy + y, cursor.colour);
}

// plot character on the framebuffer
void framebufferPlotc(char c, uint32_t x, uint32_t y)
{
    framebufferPlotc_impl(c, x, y);
}

// create a new line
ifunc void newline()
{
    cursor.Y += font->height + 1; // add character's height and a 1 px padding
    cursor.X = 0;                 // reset cursor X

    if (cursor.Y + font->height + 1 >= back.height) // we can't write further
    {
#ifdef K_FB_SCROLL
        cursor.Y -= font->height + 1; // move back to the last line

        size_t fbSize = back.pitch * back.height;        // size in bytes of the frame buffer
        size_t offset = (font->height + 1) * back.pitch; // get offset of the second line
        size_t bytes = fbSize - offset;                  // calculate the bytes to be copied

        memcpy64(back.address, back.address + offset, bytes / 8); // do the actual copy (todo: use memmove here)

        memset64(back.address + bytes, 0, (fbSize - bytes) / 8); // zero last line
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

    if (cursor.X + font->width > back.width)
        newline();

    // NOTE: here we should use the lock but, for performance concerns, we omit it...

    framebufferPlotc_impl(c, cursor.X, cursor.Y); // use inline version of framebufferPlotc
    cursor.X += font->width + 1;                  // add character's width and a 1 px padding
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
