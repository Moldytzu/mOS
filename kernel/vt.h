#pragma once
#include <utils.h>

#define VT_DISPLAY_KERNEL 0
#define VT_DISPLAY_TTY0 1
#define VT_DISPLAY_FB 2

struct vt_terminal
{
    const char *buffer; // pointer to the terminal buffer
    uint32_t bufferLen; // lenght of the buffer
    uint32_t bufferIdx; // current index in the buffer
    uint32_t id;        // ttyID

    struct vt_terminal *next; // next terminal
};

void vtSetMode(uint16_t displayMode);
uint16_t vtGetMode();
void vtAppend(struct vt_terminal *vt, const char *str, size_t count);
struct vt_terminal *vtGet(uint32_t id);
struct vt_terminal *vtRoot();
struct vt_terminal *vtCreate();