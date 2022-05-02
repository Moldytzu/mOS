#include <vt.h>
#include <heap.h>
#include <pmm.h>
#include <vmm.h>

struct vt_terminal rootTerminal;
uint32_t lastID = 0;
bool refresh = false;

struct vt_terminal *vtCreate()
{
    struct vt_terminal *currentTerminal = &rootTerminal; // first terminal

    if (currentTerminal->buffer) // check if the root terminal is valid
    {
        while (currentTerminal->next) // get last terminal
            currentTerminal = currentTerminal->next;

        if (currentTerminal->buffer)
        {
            currentTerminal->next = malloc(sizeof(struct vt_terminal)); // allocate next terminal if the current terminal is valid
            currentTerminal = currentTerminal->next;                    // set current terminal to the newly allocated terminal
        }
    }

    memset64(currentTerminal, 0, sizeof(struct vt_terminal) / sizeof(uint64_t)); // clear the terminal
    currentTerminal->buffer = mmAllocatePage();                                  // allocate the buffer
    currentTerminal->bufferLen = VMM_PAGE;                                       // set the lenght of the buffer
    currentTerminal->id = lastID++;                                              // set the ID

#ifdef K_VT_DEBUG
    printks("vt: creating new terminal with ID %d\n\r", currentTerminal->id);
#endif

    return currentTerminal;
}

void vtAppend(struct vt_terminal *vt, const char *str, size_t count)
{
    if (vt == &rootTerminal)
        refresh = true; // set refresh flag

    const char *input = str;                    // input buffer
    if (vt->bufferIdx + count >= vt->bufferLen) // check if we could overflow
    {
        input += (vt->bufferIdx + count) - vt->bufferLen;                  // move the pointer until where it overflows
        count -= (vt->bufferIdx + count) - vt->bufferLen;                  // decrease the count by the number of bytes where it overflows
        memset64((void *)vt->buffer, 0, vt->bufferLen / sizeof(uint64_t)); // clear the buffer
        vt->bufferIdx = 0;                                                 // reset the index
    }

    memcpy8((void *)((uint64_t)vt->buffer + vt->bufferIdx), (void *)input, count); // copy the buffer
    vt->bufferIdx += count;                                                        // increment the index

#ifdef K_VT_DEBUG
    printks("vt: appended %d bytes to terminal %d\n\r", count, vt->id);
#endif
}

struct vt_terminal *vtRoot()
{
    return &rootTerminal;
}

struct vt_terminal *vtGet(uint32_t id)
{
    if (id > lastID) // out of bounds
        return NULL;

    struct vt_terminal *terminal = &rootTerminal; // first terminal
    while (terminal->id != id && terminal->next)
        terminal = terminal->next;

    return terminal;
}

uint16_t mode;

void vtSetMode(uint16_t displayMode)
{
    mode = displayMode;
}

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