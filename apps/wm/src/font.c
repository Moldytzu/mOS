// project headers
#include <wm.h>
#include <font.h>

// libc
#include <mos/sys.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct __attribute__((packed)) // https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html
{
    uint8_t magic[4];
    uint32_t version;
    uint32_t headersize; /* offset of bitmaps in file */
    uint32_t flags;
    uint32_t length;        /* number of glyphs */
    uint32_t charsize;      /* number of bytes for each character */
    uint32_t height, width; /* max dimensions of glyphs */
} psf2_header_t;

psf2_header_t *font;

void fontLoad(const char *path)
{
    size_t fd = sys_open(path); // open the file
    assert(fd != 0);

    size_t size = sys_vfs(SYS_VFS_FILE_SIZE, fd, 0); // get the size
    assert(size != 0);

    uint16_t pages = size / 4096 + 1;

    font = (psf2_header_t *)sys_mem(SYS_MEM_ALLOCATE, pages, 0);
    assert(font);

    sys_read(font, size, fd); // read the file

    uint16_t pitch = font->charsize / font->height;

    printf("%p %d", font, pitch);
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);
}

void fontPlot(char c, uint16_t x, uint16_t y, uint32_t colour)
{
    uint16_t pitch = font->charsize / font->height;
    uint8_t *character = (uint8_t *)font + font->headersize + c * font->charsize; // get the offset by skipping the header and indexing the character

    for (size_t dy = 0; dy < font->height; dy++) // loop for each pixel of character
        for (size_t dx = 0; dx < font->width; dx++)
            if ((character[dy * pitch + dx / 8] >> (7 - dx % 8)) & 1) // create a bit mask then and in the current byte of the character
                if (PIXEL_IN_BOUNDS(dx + x, dy + y))
                    fbPlotPixel(dx + x, dy + y, colour);
}

void fontWriteStr(const char *str, uint16_t x, uint16_t y, uint32_t colour)
{
    for (int i = 0; str[i]; i++)
    {
        fontPlot(str[i], x, y, colour);
        x += font->width;
    }
}