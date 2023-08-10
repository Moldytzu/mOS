#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define RGB(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#define DESKTOP_BACKGROUND RGB(0, 0xA0, 0xA0)
#define DESKTOP_BACKGROUND64 (((uint64_t)DESKTOP_BACKGROUND << 32) | DESKTOP_BACKGROUND)
#define CURSOR_COLOUR RGB(0xFF, 0, 0)
#define PIXEL_IN_BOUNDS(x, y) ((x) < screenW && (y) < screenH)

uint64_t pitch, screenW, screenH;

// front buffer
uint32_t *fbStart;
uint64_t fbLen;

// back buffer
uint32_t *backStart;
uint32_t backPages;

// helper functions
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
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);
    puts(msg);
}

void updateScreenMetadata()
{
    uint64_t oldFbLen = fbLen;

    fbStart = (uint32_t *)sys_display(SYS_DISPLAY_MAP_FB, (uint64_t)&pitch, 0);
    sys_display(SYS_DISPLAY_GET, (uint64_t)&screenW, (uint64_t)&screenH);
    fbLen = pitch * screenH;

    if (oldFbLen != fbLen && oldFbLen != 0)
    {
        // fixme: well frick... we don't have deallocation routines for userspace thus we can't do reallocation of the back buffer
        // todo: we could do a fallback here to change the back to the front....
        panic("Framebuffer size mismatched.\n");
        abort();
    }
}

// framebuffer functions
void plotPixel(uint16_t x, uint16_t y, uint32_t colour)
{
    backStart[y * screenW + x] = colour;
}

// handle functions
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
    memset64(backStart, DESKTOP_BACKGROUND64, fbLen / sizeof(uint64_t)); // clear the background
}

void updateScreen()
{
    memcpy64(fbStart, backStart, fbLen / sizeof(uint64_t)); // copy the back buffer to the front one
}

// main function
int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_FB, 0);

    fbLen = 0;
    backStart = NULL;

    updateScreenMetadata();                                          // generate initial screen information
    backPages = fbLen / 4096 + 1;                                    // calculate pages required
    backStart = (uint32_t *)sys_mem(SYS_MEM_ALLOCATE, backPages, 0); // allocate the pages

    // event loop
    while (true)
    {
        updateScreenMetadata();

        handleDesktop();
        handleMouse();

        updateScreen();

        sys_yield(); // allow for screen refresh
    }
}