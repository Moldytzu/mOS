// project headers
#include <wm.h>
#include <cursor.h>

// libc
#include <mos/sys.h>
#include <stdbool.h>

// cursor coordinates
uint16_t cursorX, cursorY;

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

    // todo: maybe we could set a flag if the cursor has changed to trigger a redraw of it?
}

void cursorRedraw()
{
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
}