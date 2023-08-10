#pragma once
#include <stddef.h>
#include <stdint.h>

//
//  Macros
//

// helper macros
#define RGB(r, g, b) ((uint32_t)(((r) << 16) | ((g) << 8) | (b)))
#define CURSOR_BORDER_COLOUR RGB(0, 0, 0)
#define CURSOR_FILL_COLOUR RGB(0xFF, 0xFF, 0xFF)
#define DESKTOP_BACKGROUND64 (((uint64_t)DESKTOP_BACKGROUND << 32) | DESKTOP_BACKGROUND)
#define PIXEL_IN_BOUNDS(x, y) ((x) < screenW && (y) < screenH)

// settings
#define DESKTOP_BACKGROUND RGB(0, 0xA0, 0xA0)

//
//  Global variables
//

// global variables (implemented in wm.c)
extern uint64_t pitch, screenW, screenH;

// front buffer
extern uint32_t *fbStart;
extern uint64_t fbLen;

// back buffer
extern uint32_t *backStart;
extern uint32_t backPages;

//
//  Functions
//

// helper functions
void *memset64(void *str, uint64_t c, size_t n);
void *memcpy64(void *restrict s1, const void *restrict s2, size_t n);
void panic(const char *msg);

// framebuffer functions
inline static void fbPlotPixel(uint16_t x, uint16_t y, uint32_t colour)
{
    backStart[y * screenW + x] = colour;
}

void fbFillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t colour);