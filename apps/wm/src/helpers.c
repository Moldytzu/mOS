#include <wm.h>
#include <mos/sys.h>
#include <stdio.h>
#include <stdlib.h>

void *memset64(void *str, uint64_t c, size_t n)
{
    for (; n; n--, str += sizeof(uint64_t))
        *(uint64_t *)str = c;
    return str;
}

void *memcpy64(void *restrict s1, const void *restrict s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint64_t *)s1)[i] = ((uint64_t *)s2)[i];
    return s1;
}

void panic(const char *msg)
{
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // reset to tty (console) mode
    puts(msg);                                         // print the message
    abort();                                           // and abort the mission
}

void fbFillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t colour)
{
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            if (PIXEL_IN_BOUNDS(x + i, y + j))
                fbPlotPixel(x + i, y + j, colour);
        }
    }
}