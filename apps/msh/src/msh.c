#include <mos/sys.h>
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

    memset(arguments[0], 0, 4096); // clear the first argument

    int i = 0;
    argumentsCount = 0;
    // split the buffer
    while (*buffer)
    {
        if (*buffer == ' ') // split
        {
            argumentsCount++; // switch the buffer

            memset(arguments[argumentsCount], 0, 4096); // clear the argument

            if (argumentsCount > 32) // don't overrun
            {
                argumentsCount--;
                break;
            }

            i = 0;    // reset index
            buffer++; // skip character
            continue;
        }
        arguments[argumentsCount][i++] = *(buffer++);
    }

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
            printf("Couldn't find directory %s\n", arguments[1]);

            memset(cwdBuffer, 0, 4096); // clear cwd
            return;
        }

        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t *)cwdBuffer); // set the current working directory buffer
        return;
    }

    // append the extension if it doesn't exist
    if (memcmp(arguments[0] + strlen(arguments[0]) - 3, ".mx", 3) != 0)
        memcpy((void *)(arguments[0] + strlen(arguments[0])), ".mx\0", 4); // copy the extension (including the NULL)

    memset((void *)cmdBuffer, 0, 4096);                            // clear the buffer
    memcpy((void *)cmdBuffer, arguments[0], strlen(arguments[0])); // copy the input

    uint64_t status = sys_open(arguments[0]);

    if (!status)
    {
        // try to append the path
        memset((void *)cmdBuffer, 0, 4096);
        memcpy((void *)(cmdBuffer + pathLen), arguments[0], strlen(arguments[0])); // copy the input
        memcpy((void *)cmdBuffer, path, pathLen);                                  // copy the path

        uint64_t status = sys_open(cmdBuffer);
        if (status && strlen(cmdBuffer))
            goto execute;

        printf("Couldn't find executable %s\n", arguments[0]);
        return;
    }

execute:
    uint64_t newPid;
    char *argv[31];
    argumentsCount = min(30, argumentsCount); // clamp the value to 30 arguments
    for (int i = 0; i < argumentsCount; i++)
        argv[i] = arguments[i + 1];
    sys_exec_packet_t p = {0, enviroment, cwdBuffer, argumentsCount, argv};
    sys_exec(cmdBuffer, &newPid, &p);
    do
    {
        sys_pid(newPid, SYS_PID_STATUS, &status); // get the status of the pid
    } while (status == 0);                        // wait for the pid to be stopped
}

int main(int argc, char **argv)
{
    puts("m Shell\n");

    // keyboard buffer
    char *kBuffer = malloc(4096);
    uint16_t kIdx;
    assert(kBuffer != NULL); // assert that the buffer is valid

    // enviroment buffer
    enviroment = malloc(4096);
    assert(enviroment != NULL); // assert that the enviroment is valid

    // command buffer
    cmdBuffer = malloc(4096);
    assert(cmdBuffer != NULL); // assert that the buffer is valid

    // current working directory buffer
    cwdBuffer = malloc(512);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    pid = sys_pid_get();
    sys_pid(pid, SYS_PID_GET_ENVIROMENT, (uint64_t *)enviroment); // get the enviroment

    if (argc >= 2)
        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t *)argv[1]); // set the current working directory to the argument

    // allocate the arguments buffers
    for (int i = 0; i < 32; i++)
    {
        arguments[i] = malloc(4096);
        assert(arguments[i] != NULL); // assert that the buffer is valid
    }

    // set the start
    path = enviroment;

    // find the path
    while (memcmp(path, "PATH=", 5) != 0 && strlen(path)) // find the path in enviroment
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

        printf("%s m$ ", cwdBuffer); // print the prompt

        // read in the buffer
        do
        {
            sys_input(SYS_INPUT_KEYBOARD, &chr); // read a character off the keyboard buffer
            
            if(chr == '\b') // handle backspace
            {
                if(!kIdx)
                    continue;

                kBuffer[--kIdx] = 0;
                printf(" -> %s", kBuffer); // print the prompt
                continue;
            }
            
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