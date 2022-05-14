#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

uint16_t pathLen = 0;
const char *path;

const char *cmdBuffer;

void handleInput(const char *buffer)
{
    if (strcmp(buffer, "exit") == 0) // exit command
        exit(EXIT_SUCCESS);

    memset((void *)cmdBuffer, 0, 4096);                            // clear the buffer
    memcpy((void *)(cmdBuffer + pathLen), buffer, strlen(buffer)); // copy the input
    if (pathLen)
        memcpy((void *)cmdBuffer, path, pathLen); // copy the path

    uint64_t pid, status;
    sys_exec(cmdBuffer, 0, &pid, 0);

    do
    {
        sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
    } while (status == 0);                     // wait for the pid to be stopped
}

int main()
{
    puts("m Shell\n");

    // keyboard buffer
    char *kBuffer;
    uint16_t kIdx;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&kBuffer, 0);
    assert(kBuffer != NULL); // assert that the buffer is valid

    // enviroment buffer
    const char *enviroment;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&enviroment, 0);
    assert(enviroment != NULL); // assert that the enviroment is valid

    // command buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cmdBuffer, 0);
    assert(cmdBuffer != NULL); // assert that the buffer is valid

    uint64_t PID;
    sys_pid(0, SYS_PID_GET, &PID);                                // get the pid
    sys_pid(PID, SYS_PID_GET_ENVIROMENT, (uint64_t *)enviroment); // get the enviroment

    // set the start
    path = enviroment;

    // find the path
    while (memcmp(path, "PATH=", 5) != 0) // find the path in enviroment
        path++;

    if (memcmp(path, "PATH=", 5) == 0) // check if the path is available by comparing again the bytes
    {
        path += 5; // skip the PATH= part
        for (pathLen = 0; path[pathLen] != '|'; pathLen++)
            ; // calculate the path len
    }

    // print the path just for debugging purposes
    puts("The path is ");
    for (int i = 0; i < pathLen; i++)
        putchar(path[i]);
    putchar('\n');

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