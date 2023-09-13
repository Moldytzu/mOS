#include <vt/vt.h>
#include <mm/pmm.h>
#include <mm/blk.h>
#include <mm/vmm.h>
#include <misc/logger.h>

struct vt_terminal rootTerminal;
uint32_t lastID = 0;
bool refresh = false;

// create new terminal and return it's address
struct vt_terminal *vtCreate()
{
    struct vt_terminal *currentTerminal = &rootTerminal; // first terminal

    if (currentTerminal->buffer) // it is valid when the first terminal was allocated
    {
        while (currentTerminal->next) // get last terminal
            currentTerminal = currentTerminal->next;

        currentTerminal->next = blkBlock(sizeof(struct vt_terminal)); // allocate next terminal if the current terminal is valid
        currentTerminal->next->previous = currentTerminal;            // set the previous terminal
        currentTerminal = currentTerminal->next;                      // set current terminal to the newly allocated terminal
    }

    currentTerminal->buffer = pmmPage();   // allocate the character buffer
    currentTerminal->kbBuffer = pmmPage(); // allocate the keyboard buffer
    currentTerminal->id = lastID++;        // set the ID
    currentTerminal->bufferPages = 1;

#ifdef K_VT_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vt: creating new terminal with ID %d", currentTerminal->id);
#endif

    return currentTerminal;
}

// append text on a terminal
void vtAppend(struct vt_terminal *vt, const char *str, size_t count)
{
    if (!count || !vt)
        return;

    lock(vt->lock, {
        const char *input = str; // input buffer

        if (vt->bufferIdx + count >= VMM_PAGE * vt->bufferPages) // reallocate if we overflow
        {
            // calculate required indices
            // NOTE: without the volatile keyword the compiler will f-up the values
            volatile size_t oldPages = vt->bufferPages;
            volatile size_t newPages = ++vt->bufferPages;

            vt->buffer = pmmReallocate((void *)vt->buffer, oldPages, newPages); // perform the reallocation

            logDbg(LOG_SERIAL_ONLY, "vt: reallocating buffer of id %d to %d pages", vt->id, vt->bufferPages);
        }

        for (size_t i = 0; i < count; i++) // copy the buffer
        {
            if (*input == '\b') // handle ascii backspace
            {
                vt->buffer[--vt->bufferIdx] = '\0'; // zero the buffer early
                continue;
            }

            vt->buffer[vt->bufferIdx++] = *(input++); // set the byte
        }
    });

    if (vt == &rootTerminal)
        refresh = true; // set refresh flag

#ifdef K_VT_DEBUG
    logDbg(LOG_SERIAL_ONLY, "vt: appended %d bytes to terminal %d", count, vt->id);
#endif
}

// get first terminal
struct vt_terminal *vtRoot()
{
    return &rootTerminal;
}

// get terminal with id
struct vt_terminal *vtGet(uint32_t id)
{
    if (id > lastID) // out of bounds
        return NULL;

    struct vt_terminal *terminal = &rootTerminal; // first terminal
    while (terminal->id != id && terminal->next)
        terminal = terminal->next;

    return terminal;
}

// append a key to the keyboard buffer
void vtkbAppend(struct vt_terminal *vt, char c)
{
    if (!vt)
        return;

    lock(vt->lock, {
        vt->kbBuffer[vt->kbBufferIdx++] = c; // append the character

        if (vt->kbBufferIdx == 4096) // prevent buffer overflow
            vt->kbBufferIdx = 0;
    });
}

// get firstly typed key from the keyboard buffer
char vtkbGet(struct vt_terminal *vt)
{
    if (!vt->kbBufferIdx || !vt) // we don't have anything to return
        return 0;

    char first = vt->kbBuffer[0];                             // get the key
    memmove(vt->kbBuffer, vt->kbBuffer + 1, vt->kbBufferIdx); // shift the buffer to the left

    return first;
}

uint16_t mode;

// set the mode of displaying things
void vtSetMode(uint16_t displayMode)
{
    mode = displayMode;
}

// get the mode
uint16_t vtGetMode()
{
#ifdef K_FB_TTY_REFRESH_ON_DEMAND
    if (mode != VT_DISPLAY_TTY0)
        return mode;

    if (refresh) // if the refresh flag is set then we have to refresh the tty
    {
        refresh = false;
        return mode; // return the mode
    }

    return 0; // return the null mode (kernel mode)
#else
    return mode;
#endif
}

// free the terminal
void vtDestroy(struct vt_terminal *vt)
{
    if (!vt)
        return;

    if (vt->previous)
        vt->previous->next = vt->next; // bypass this node (if we can)

    pmmDeallocatePages((void *)vt->buffer, vt->bufferPages); // deallocate the buffer
    pmmDeallocate((void *)vt->kbBuffer);                     // deallocate the keyboard buffer
    blkDeallocate(vt);                                       // free the terminal
}