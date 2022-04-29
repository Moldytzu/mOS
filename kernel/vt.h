#pragma once
#include <utils.h>

struct vt_terminal
{
    const char *buffer; // pointer to the terminal buffer
    uint32_t bufferLen; // lenght of the buffer
    uint32_t bufferIdx; // current index in the buffer
    uint32_t id;        // ttyID

    struct vt_terminal *next; // next terminal
};

struct vt_terminal *vtRoot();
struct vt_terminal *vtCreate();