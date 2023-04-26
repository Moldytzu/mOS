#pragma once
#include <misc/utils.h>

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

pstruct // https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
{
    uint8_t magic[4];
    uint32_t version;
    uint32_t headersize; /* offset of bitmaps in file */
    uint32_t flags;
    uint32_t length;        /* number of glyphs */
    uint32_t charsize;      /* number of bytes for each character */
    uint32_t height, width; /* max dimensions of glyphs */
}
psf2_header_t;

pstruct
{
    uint32_t X, Y, colour;
}
framebuffer_cursor_info_t;

void framebufferBenchmark();
void framebufferInit();
void framebufferFlush();
void framebufferClear(uint32_t colour);
void framebufferWrite(const char *str);
void framebufferWritec(char c);
framebuffer_cursor_info_t framebufferGetCursor();
void framebufferSetCursor(framebuffer_cursor_info_t info);