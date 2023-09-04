// project headers
#include <wm.h>
#include <cursor.h>

// libc
#include <mos/sys.h>
#include <stdbool.h>

// cursor coordinates
uint16_t cursorX, cursorY;

uint16_t lastCursorX, lastCursorY;

// todo: replace this with an actual image stored on the disk
uint8_t cursorImage[8][8] = {
    {1, 1, 1, 1, 1, 0, 0, 0},
    {1, 2, 2, 2, 1, 0, 0, 0},
    {1, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 1, 2, 1, 0, 0, 0},
    {1, 1, 0, 1, 2, 1, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

static const uint8_t cursorScale = 4;

#define bittest(bmp, bit) ((0b10000000 >> bit) & bmp)

void cursorUpdate()
{
    sys_input_mouse(&cursorX, &cursorY); // read mouse coordinates
}

void cursorRedraw()
{
    // todo: find a way to not use the double buffer

    uint32_t underCursor[8 * cursorScale][8 * cursorScale];

    // save framebuffer colours in the point where the cursor will be placed
    for (int y = 0; y < 8 * cursorScale; y++)
        for (int x = 0; x < 8 * cursorScale; x++)
            if (PIXEL_IN_BOUNDS(cursorX + x, cursorY + y))
                underCursor[x][y] = fbGetPixel(cursorX + x, cursorY + y);

    // draw the cursor
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            switch (cursorImage[y][x])
            {
            case 1:
                fbFillRectangle(cursorX + x * cursorScale, cursorY + y * cursorScale, cursorScale, cursorScale, CURSOR_BORDER_COLOUR);
                break;

            case 2:
                fbFillRectangle(cursorX + x * cursorScale, cursorY + y * cursorScale, cursorScale, cursorScale, CURSOR_FILL_COLOUR);
                break;

            default:
                break;
            }
        }
    }

    sys_display(SYS_DISPLAY_UPDATE_FB, 0, 0); // update framebuffer

    // restore modified pixels
    for (int y = 0; y < 8 * cursorScale; y++)
        for (int x = 0; x < 8 * cursorScale; x++)
            if (PIXEL_IN_BOUNDS(cursorX + x, cursorY + y))
                fbPlotPixel(cursorX + x, cursorY + y, underCursor[x][y]);
}