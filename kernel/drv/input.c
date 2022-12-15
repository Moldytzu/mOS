#include <drv/input.h>
#include <subsys/vt.h>
#include <sched/scheduler.h>

struct vt_terminal *startTerminal, *currentTerminal;

drv_type_input_t drv_type_input_s;

// todo: implement mouse support

// deprecated: append a char on the buffers (this will be removed)
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

    zero(&drv_type_input_s, sizeof(drv_type_input_s)); // clear the context
}

// flush struct changes
void inputFlush()
{
    // flush the newly appended keys to the terminal buffers
    for (int i = 0; i < min(strlen(drv_type_input_s.keys), 16); i++)
    {
        char c = drv_type_input_s.keys[i];

        // append the key to every terminal in a loop
        currentTerminal = startTerminal;
        while (currentTerminal)
        {
            vtkbAppend(currentTerminal, c);
            currentTerminal = currentTerminal->next;
        }
    }

    zero(drv_type_input_s.keys, 16); // clear the buffer
}