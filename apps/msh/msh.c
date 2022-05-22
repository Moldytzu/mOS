#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

uint64_t pid;
uint16_t pathLen = 0;
const char *path;
const char *enviroment;
const char *cmdBuffer;
char *cwdBuffer;
char *arguments[32];
int argumentsCount = 0;

void handleInput(const char *buffer)
{
    if (!*buffer) // empty input
        return;

    int i = 0;
    argumentsCount = 0;
    // split the buffer
    while (*buffer)
    {
        if (*buffer == ' ') // split
        {
            argumentsCount++; // switch the buffer

            if (argumentsCount > 32) // don't overrun
            {
                argumentsCount--;
                break;
            }

            arguments[argumentsCount][i] = 0; // terminate the argument
            i = 0;                            // reset index
            buffer++;                         // skip character
            continue;
        }
        arguments[argumentsCount][i++] = *(buffer++);
    }
    arguments[argumentsCount][i++] = 0; // terminate the argument

    if (strcmp(arguments[0], "exit") == 0) // exit command
        exit(EXIT_SUCCESS);

    if (strcmp(arguments[0], "cd") == 0) // change directory command
    {
        if (argumentsCount != 1)
        {
            puts("Usage: cd <directory>\n");
            return;
        }

        if (strcmp(arguments[1], "..") == 0) // go back a folder
        {
            for (int i = strlen(cwdBuffer) - 2; cwdBuffer[i] != '/'; cwdBuffer[i--] = '\0')
                ; // step back to last delimiter
        }
        else if (*arguments[1] == '/') // full path
        {
            memset(cwdBuffer, 0, 4096);                            // clear the buffer
            memcpy(cwdBuffer, arguments[1], strlen(arguments[1])); // copy the buffer
        }
        else
        {
            if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
                cwdBuffer[strlen(cwdBuffer)] = '/';

            memcpy(cwdBuffer + strlen(cwdBuffer), arguments[1], strlen(arguments[1])); // copy the buffer
        }

        if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
            cwdBuffer[strlen(cwdBuffer)] = '/';

        uint64_t status;
        sys_vfs(SYS_VFS_DIRECTORY_EXISTS, (uint64_t)cwdBuffer, (uint64_t)&status); // check if the directory exists

        if (!status)
        {
            puts("Couldn't find directory ");
            puts(arguments[1]);
            putchar('\n');

            // restore the cwd
            sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer
            return;
        }

        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t *)cwdBuffer); // set the current working directory buffer
        return;
    }

    // append the extension if it doesn't exist
    if (memcmp(arguments[0] + strlen(arguments[0]) - 3, ".mx", 3) != 0)
        memcpy((void *)(arguments[0] + strlen(arguments[0])), ".mx", 3); // copy the extension

    uint64_t status;
    sys_open(arguments[0], &status);
    if (!status)
    {
        puts("Couldn't find executable ");
        puts(arguments[0]);
        putchar('\n');
        return;
    }

    uint64_t newPid;
    char *argv[31];
    for (int i = 0; i < argumentsCount; i++)
        argv[i] = arguments[i + 1];
    struct sys_exec_packet p = {0, enviroment, cwdBuffer, argumentsCount, argv};
    sys_exec(arguments[0], &newPid, &p);

    do
    {
        sys_pid(newPid, SYS_PID_STATUS, &status); // get the status of the pid
    } while (status == 0);                        // wait for the pid to be stopped
}

int main(int argc, char **argv)
{
    puts("m Shell\n");

    // keyboard buffer
    char *kBuffer;
    uint16_t kIdx;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&kBuffer, 0);
    assert(kBuffer != NULL); // assert that the buffer is valid

    // enviroment buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&enviroment, 0);
    assert(enviroment != NULL); // assert that the enviroment is valid

    // command buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cmdBuffer, 0);
    assert(cmdBuffer != NULL); // assert that the buffer is valid

    // current working directory buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cwdBuffer, 0);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET, &pid);                                // get the pid
    sys_pid(pid, SYS_PID_GET_ENVIROMENT, (uint64_t *)enviroment); // get the enviroment

    // allocate the arguments buffers
    for (int i = 0; i < 32; i++)
    {
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&arguments[i], 0);
        assert(arguments[i] != NULL); // assert that the buffer is valid
    }

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

    // main loop
    while (1)
    {
        sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer
        memset(kBuffer, 0, 4096);                             // clear the buffer

        char chr;

        puts(cwdBuffer);
        puts(" m$ "); // print the prompt

        // read in the buffer
        do
        {
            sys_input(SYS_INPUT_KEYBOARD, &chr); // read a character off the keyboard buffer
            if (chr)
            {
                putchar(chr);
                kBuffer[kIdx++] = chr; // append the character
            }

            if (kIdx == 4095) // prevent buffer overflow
            {
                kBuffer[4095] = 0;
                break;
            }

        } while (chr != '\n');
        kBuffer[--kIdx] = 0; // terminate the string
        kIdx = 0;

        handleInput(kBuffer); // handle the input
    }
}