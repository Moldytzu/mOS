#pragma once
#include <misc/utils.h>

#define VT_DISPLAY_KERNEL 0
#define VT_DISPLAY_TTY0 1
#define VT_DISPLAY_FB 2
#define VT_DISPLAY_FB_DOUBLE_BUFFERED 3

struct vt_terminal
{
    uint32_t id;          // ID
    const char *buffer;   // pointer to the terminal buffer
    uint16_t bufferPages; // currently allocated pages
    uint16_t bufferIdx;   // current index in the buffer
    char *kbBuffer;       // pointer to the keyboard buffer
    uint16_t kbBufferIdx; // current index in the keyboard buffer
    spinlock_t lock;      // spinlock

    struct vt_terminal *previous; // previous terminal
    struct vt_terminal *next;     // next terminal
};

void vtDestroy(struct vt_terminal *vt);
void vtSetMode(uint16_t displayMode);
uint16_t vtGetMode();
void vtkbAppend(struct vt_terminal *vt, char c);
char vtkbGet(struct vt_terminal *vt);
void vtAppend(struct vt_terminal *vt, const char *str, size_t count);
struct vt_terminal *vtGet(uint32_t id);
struct vt_terminal *vtRoot();
struct vt_terminal *vtCreate();