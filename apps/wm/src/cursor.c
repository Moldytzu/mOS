// project headers
#include <wm.h>
#include <cursor.h>

// libc
#include <mos/sys.h>

// cursor coordinates
uint16_t cursorX, cursorY;

void cursorUpdate()
{
    sys_input_mouse(&cursorX, &cursorY); // read mouse coordinates

    // todo: maybe we could set a flag if the cursor has changed to trigger a redraw of it?
}

void cursorRedraw()
{
    // fixme: this is far too primitive looking... make this draw a bitmap instead
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 10; y++)
            if (PIXEL_IN_BOUNDS(cursorX + x, cursorY + y))
                plotPixel(cursorX + x, cursorY + y, CURSOR_COLOUR);
}