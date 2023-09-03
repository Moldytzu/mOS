#include <wm.h>
#include <desktop.h>

void desktopRedraw()
{
    memset64(fbStart, DESKTOP_BACKGROUND64, fbLen / sizeof(uint64_t)); // clear the background
}
