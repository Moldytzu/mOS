#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void handleInput(const char *buffer)
{
    if (strcmp(buffer, "exit") == 0) // exit command
        exit(EXIT_SUCCESS);

    uint64_t pid, status;
    sys_exec(buffer, 0, &pid);

    do
    {
        sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
    } while (status == 0);                     // wait for the pid to be stopped
}

int main()
{
    puts("m Shell\n");

    char *kBuffer;
    uint16_t kIdx;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&kBuffer, 0);
    assert(kBuffer != NULL); // assert that the buffer is valid

    // main loop
    while (1)
    {
        memset(kBuffer, 0, 4096); // clear the buffer

        char chr;

        puts("m$ "); // print the prompt

        // read in the buffer
        do
        {
            sys_input(SYS_INPUT_KEYBOARD, &chr); // read a character off the keyboard buffer
            if (chr)
            {
                putchar(chr);
                kBuffer[kIdx++] = chr; // append the character
            }
        } while (chr != '\n');
        kBuffer[--kIdx] = 0; // terminate the string
        kIdx = 0;

        handleInput(kBuffer); // handle the input
    }
}