#include <subsys/vt.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

struct vt_terminal rootTerminal;
uint32_t lastID = 0;
bool refresh = false;

// create new terminal and return it's address
struct vt_terminal *vtCreate()
{
    struct vt_terminal *currentTerminal = &rootTerminal; // first terminal

    if (currentTerminal->buffer) // check if the root terminal is valid
    {
        while (currentTerminal->next) // get last terminal
            currentTerminal = currentTerminal->next;

        if (currentTerminal->buffer)
        {
            currentTerminal->next = pmmPage(sizeof(struct vt_terminal)); // allocate next terminal if the current terminal is valid
            currentTerminal->next->previous = currentTerminal;           // set the previous terminal
            currentTerminal = currentTerminal->next;                     // set current terminal to the newly allocated terminal
        }
    }

    zero(currentTerminal, sizeof(struct vt_terminal)); // clear the terminal
    currentTerminal->buffer = pmmPage();               // allocate the character buffer
    currentTerminal->kbBuffer = pmmPage();             // allocate the keyboard buffer
    zero((void *)currentTerminal->buffer, VMM_PAGE);   // clear the character buffer
    zero((void *)currentTerminal->kbBuffer, VMM_PAGE); // clear the keyboard buffer
    currentTerminal->id = lastID++;                    // set the ID

#ifdef K_VT_DEBUG
    printks("vt: creating new terminal with ID %d\n\r", currentTerminal->id);
#endif

    return currentTerminal;
}

// append text on a terminal
void vtAppend(struct vt_terminal *vt, const char *str, size_t count)
{
    if (!count || !vt)
        return;

    if (vt == &rootTerminal)
        refresh = true; // set refresh flag

    const char *input = str;               // input buffer
    if (vt->bufferIdx + count >= VMM_PAGE) // check if we could overflow
    {
        zero((void *)vt->buffer, VMM_PAGE); // clear the buffer
        vt->bufferIdx = 0;                  // reset the index
    }

    memcpy8((void *)((uint64_t)vt->buffer + vt->bufferIdx), (void *)input, count); // copy the buffer
    vt->bufferIdx += count;                                                        // increment the index

#ifdef K_VT_DEBUG
    printks("vt: appended %d bytes to terminal %d\n\r", count, vt->id);
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

    vt->kbBuffer[vt->kbBufferIdx++] = c; // append the character

    if (vt->kbBufferIdx == 4096) // prevent buffer overflow
        vt->kbBufferIdx = 0;
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
    if (mode != VT_DISPLAY_TTY0)
        return mode;

    if (refresh) // if the refresh flag is set then we have to refresh the tty
    {
        refresh = false;
        return mode; // return the mode
    }

    return 0; // return the null mode (kernel mode)
}

// free the terminal
void vtDestroy(struct vt_terminal *vt)
{
    if (!vt)
        return;

    if (vt->previous)
        vt->previous->next = vt->next; // bypass this node (if we can)

    pmmDeallocate((void *)vt->buffer);   // deallocate the buffer
    pmmDeallocate((void *)vt->kbBuffer); // deallocate the buffer
    pmmDeallocate(vt);                   // free the terminal
}