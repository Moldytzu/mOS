#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#define DESKTOP_BACKGROUND RGB(0, 0xA0, 0xA0)
#define CURSOR_COLOUR RGB(0xFF, 0, 0)
#define PIXEL_IN_BOUNDS(x, y) ((x) < screenW && (y) < screenH)

uint64_t pitch, screenW, screenH;
uint32_t *fbStart;
uint64_t fbLen;

void *memset32(void *str, uint32_t c, size_t n)
{
    for (; n; n--, str += sizeof(uint32_t))
        *(uint32_t *)str = c;
    return str;
}

void updateScreenMetadata()
{
    fbStart = (uint32_t *)sys_display(SYS_DISPLAY_MAP_FB, (uint64_t)&pitch, 0);
    sys_display(SYS_DISPLAY_GET, (uint64_t)&screenW, (uint64_t)&screenH);
    fbLen = pitch * screenH;
}

void plotPixel(uint16_t x, uint16_t y, uint32_t colour)
{
    fbStart[y * screenW + x] = colour;
}

void handleMouse()
{
    uint16_t mouseX, mouseY;
    sys_input_mouse(&mouseX, &mouseY); // read mouse coordinates

    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 10; y++)
            if (PIXEL_IN_BOUNDS(mouseX + x, mouseY + y))
                plotPixel(mouseX + x, mouseY + y, CURSOR_COLOUR);
}

void handleDesktop()
{
    memset32(fbStart, DESKTOP_BACKGROUND, fbLen / sizeof(uint32_t)); // clear the background
}

int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_FB, 0);

    // event loop
    while (true)
    {
        updateScreenMetadata();

        handleDesktop();
        handleMouse();

        sys_yield(); // allow for screen refresh
    }
}