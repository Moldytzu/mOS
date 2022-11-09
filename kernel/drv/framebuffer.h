#pragma once
#include <misc/utils.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF1_MODE512 0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE 0x05

#define PSF1_SEPARATOR 0xFFFF
#define PSF1_STARTSEQ 0xFFFE

pstruct // https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
{
    uint8_t magic[2]; // magic number
    uint8_t mode;     // PSF font mode
    uint8_t charsize; // character size
}
psf1_header_t;

pstruct
{
    uint32_t X, Y, colour;
}
framebuffer_cursor_info_t;

void framebufferInit();
void framebufferClear(uint32_t colour);
void framebufferLoadFont(const char *name);
void framebufferPlotp(uint32_t x, uint32_t y, uint32_t colour);
void framebufferPlotc(char c, uint32_t x, uint32_t y);
void framebufferWrite(const char *str);
void framebufferWritec(char c);
framebuffer_cursor_info_t framebufferGetCursor();
void framebufferSetCursor(framebuffer_cursor_info_t info);