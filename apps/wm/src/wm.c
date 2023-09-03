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
#include <ctype.h>

// implementation of global variables
uint64_t pitch, screenW, screenH;

// display framebuffer
uint32_t *fbStart;
uint64_t fbLen;

void screenMetadataUpdate()
{
    uint64_t oldFbLen = fbLen;

    fbStart = (uint32_t *)sys_display(SYS_DISPLAY_MAP_FB, (uint64_t)&pitch, 0);
    sys_display(SYS_DISPLAY_GET, (uint64_t)&screenW, (uint64_t)&screenH);
    fbLen = pitch * screenH;
}

// main function
int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    fontLoad("hack.psf");                                             // load the hack font from initrd
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_FB_DOUBLE_BUFFERED, 0); // switch to double buffered framebuffer mode

    void *fpsBuffer = (void *)sys_mem(SYS_MEM_ALLOCATE, 1, 0);
    screenMetadataUpdate(); // generate initial screen information

    mailbox_t *mail = sys_mailbox_read();
    if (mail)
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);
        printf("%d sent us %d %s", mail->sender, mail->subject, mail->message);
        abort();
    }

    size_t fps = 0;

    // event loop
    while (true)
    {
        size_t a = sys_time_uptime_nanos();

        if (tolower(sys_input_keyboard()) == 'q')
            panic("Enjoy shell!\n");

        screenMetadataUpdate(); // look for changes of framebuffer

        cursorUpdate(); // update cursor position

        desktopRedraw(); // redraw the desktop

        // draw watermarks
        sprintf(fpsBuffer, "mOS Desktop (FPS: %d)", fps);
        fontWriteStr("Press 'Q' to exit to shell.", 10, 10, RGB(0, 0xFF, 0xFF));
        fontWriteStr(fpsBuffer, 10, screenH - 20, RGB(0xFF, 0xFF, 0));

        cursorRedraw(); // redraw the cursor

        sys_display(SYS_DISPLAY_UPDATE_FB, 0, 0); // update framebuffer

        sys_yield();

        // calculate delta time based on the time points
        size_t b = sys_time_uptime_nanos();
        size_t dt = b - a;
        fps = 100000000 / dt;
    }
}