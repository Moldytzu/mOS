#include <vt.h>
#include <heap.h>
#include <pmm.h>
#include <vmm.h>

struct vt_terminal rootTerminal;
uint32_t lastID = 0;

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

    return currentTerminal;
}

void vtAppend(struct vt_terminal *vt, const char *str, size_t count);
{

}

struct vt_terminal *vtRoot()
{
    return &rootTerminal;
}

struct vt_terminal *vtGet(uint32_t id)
{
    if (id > lastID) // out of bounds
        return NULL;

    struct vt_terminal *terminal = &rootTerminal;              // first terminal
    while ((terminal->id != id || terminal) && terminal->next) // get the terminal with the respective terminal ID
        terminal = terminal->next;

    return terminal;
}