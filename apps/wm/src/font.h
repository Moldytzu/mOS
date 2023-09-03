#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct __attribute__((packed)) // https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
{
    uint8_t magic[4];
    uint32_t version;
    uint32_t headersize; /* offset of bitmaps in file */
    uint32_t flags;
    uint32_t length;        /* number of glyphs */
    uint32_t charsize;      /* number of bytes for each character */
    uint32_t height, width; /* max dimensions of glyphs */
} psf2_header_t;

void fontLoad(const char *path);
void fontPlot(char c, uint16_t x, uint16_t y, uint32_t colour);
void fontWriteStr(const char *str, uint16_t x, uint16_t y, uint32_t colour);