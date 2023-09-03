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

size_t fps = 0;

void screenMetadataUpdate()
{
    size_t oldScreenW = screenW, oldScreenH = screenH;

    sys_display(SYS_DISPLAY_GET, (uint64_t)&screenW, (uint64_t)&screenH); // get current screen resolution

    if (oldScreenH != screenH || oldScreenW != screenW) // screen resolution changed
    {
        fbStart = (uint32_t *)sys_display(SYS_DISPLAY_MAP_FB, (uint64_t)&pitch, 0); // remap framebuffer
        fbLen = pitch * screenH;
    }
}

void handleKeyboard()
{
    char lastKey = sys_input_keyboard();
    if (tolower(lastKey) == 'q')
        panic("Enjoy shell!\n");
}

extern psf2_header_t *font;

void draw()
{
    // draw watermarks
    fontWriteStr("Press 'Q' to exit to shell.", 10, 10, RGB(0, 0xFF, 0xFF));

    // fixme: refresh on demand
    char fpsBuffer[64];
    sprintf(fpsBuffer, "mOS Desktop (FPS: %d)", fps);
    fbFillRectangle(10, screenH - 20, strlen(fpsBuffer) * font->width, font->height, DESKTOP_BACKGROUND);
    fontWriteStr(fpsBuffer, 10, screenH - 20, RGB(0xFF, 0xFF, 0));
}

// main function
int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    fontLoad("hack.psf");                                             // load the hack font from initrd
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_FB_DOUBLE_BUFFERED, 0); // switch to double buffered framebuffer mode
    screenMetadataUpdate();                                           // generate initial screen information

    // read last mail
    mailbox_t *mail = sys_mailbox_read();
    if (mail)
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);
        printf("%d sent us %d %s", mail->sender, mail->subject, mail->message);
        abort();
    }

    // todo: we need a syncronization call inside display syscall handler to wait for all threads to get to queue start
    // in some situations here we might start as a thread on an application core
    // which could mean the bsp could still write to the framebuffer
    // and this creates a race condition in which the tty contents will still be written to the screen without us knowing
    for (int i = 0; i < 100; i++)
        sys_yield();

    desktopRedraw(); // redraw the desktop

    // event loop
    while (true)
    {
        size_t a = sys_time_uptime_nanos(); // first time point

        screenMetadataUpdate(); // look for changes of framebuffer
        handleKeyboard();       // handle key presses

        cursorUpdate(); // update cursor position
        draw();         // do the drawing if necessary
        cursorRedraw(); // redraw cursor and update screen

        sys_yield();

        // calculate delta time based on the time points
        size_t b = sys_time_uptime_nanos();
        size_t dt = b - a;
        fps = 100000000 / dt;
    }
}