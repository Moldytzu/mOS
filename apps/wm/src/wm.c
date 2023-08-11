// project headers
#include <wm.h>
#include <cursor.h>
#include <desktop.h>
#include <font.h>

// libc
#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// implementation of global variables
uint64_t pitch, screenW, screenH;

// front buffer
uint32_t *fbStart;
uint64_t fbLen;

// back buffer
uint32_t *backStart;
uint32_t backPages;

void screenMetadataUpdate()
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
    }
}

void screenUpdate()
{
    memcpy64(fbStart, backStart, fbLen / sizeof(uint64_t)); // copy the back buffer to the front one
}

// main function
int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    fontLoad("hack.psf");                             // load the hack font from initrd
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_FB, 0); // switch to framebuffer mode

    void *fpsBuffer = (void *)sys_mem(SYS_MEM_ALLOCATE, 1, 0);

    // initialise variables
    fbLen = 0;
    backStart = NULL;

    screenMetadataUpdate();                                          // generate initial screen information
    backPages = fbLen / 4096 + 1;                                    // calculate pages required
    backStart = (uint32_t *)sys_mem(SYS_MEM_ALLOCATE, backPages, 0); // allocate the pages

    size_t fps = 0;

    // event loop
    while (true)
    {
        size_t a = sys_time_uptime_nanos();

        screenMetadataUpdate(); // look for changes of framebuffer

        cursorUpdate(); // update cursor position

        desktopRedraw(); // redraw the desktop

        // draw watermark
        sprintf(fpsBuffer, "mOS Desktop (FPS: %d)", fps);
        fontWriteStr(fpsBuffer, 10, 10, RGB(0, 0, 0));

        cursorRedraw(); // redraw the cursor

        screenUpdate(); // update the screen

        sys_yield(); // allow for screen refresh

        // calculate delta time based on the time points
        size_t b = sys_time_uptime_nanos();
        size_t dt = b - a;
        fps = 100000000 / dt;
    }
}