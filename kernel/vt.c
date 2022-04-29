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
            currentTerminal->next = malloc(sizeof(struct vfs_terminal)); // allocate next terminal if the current terminal is valid
            currentTerminal = currentTerminal->next;                     // set current terminal to the newly allocated terminal
        }
    }

    memset64(currentTerminal, 0, sizeof(struct vt_terminal) / sizeof(uint64_t)); // clear the terminal
    currentTerminal->buffer = mmAllocatePage();                                  // allocate the buffer
    currentTerminal->bufferLen = VMM_PAGE;                                       // set the lenght of the buffer
    currentTerminal->id = lastID++;                                              // set the ID

    return currentTerminal;
}

struct vt_terminal *vtRoot()
{
    return &rootTerminal;
}