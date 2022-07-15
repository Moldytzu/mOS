#include <subsys/input.h>
#include <subsys/vt.h>
#include <sched/scheduler.h>

struct vt_terminal *startTerminal, *currentTerminal;

// append a char on the buffers
void kbAppendChar(char c)
{
    // loop thru each terminal and then append the character to each of them
    currentTerminal = startTerminal;
    while (currentTerminal)
    {
        vtkbAppend(currentTerminal, c);
        currentTerminal = currentTerminal->next;
    }
}

// pop last key 
char kbGetLastKey()
{
    return vtkbGet(vtGet(schedulerGetCurrent()->terminal)); // get last key of the terminal
}

// get the buffer
char *kbGetBuffer()
{
    return (char *)vtGet(schedulerGetCurrent()->terminal)->buffer; // get the buffer
}

// initialize the subsystem
void inputInit()
{
    startTerminal = vtGet(0);
}