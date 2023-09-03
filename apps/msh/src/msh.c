#include <mos/sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ARGUMENTS 31

uint64_t pid = 0;
char *path = NULL;
char *enviroment = NULL;
char *cmdBuffer = NULL;
char *cwdBuffer = NULL;
char *arguments[MAX_ARGUMENTS];
int argumentsCount = 0;

void handleInput(const char *buffer)
{
    if (!*buffer) // empty input
        return;

    memset(arguments[0], 0, 4096); // clear the first argument

    int i = 0;
    argumentsCount = 0;

    // split the buffer at every space
    while (*buffer)
    {
        if (*buffer == ' ') // when we hit a space then we switch to the next argument buffer
        {
            argumentsCount++; // increment the count of arguments

            if (argumentsCount > MAX_ARGUMENTS) // don't overrun
            {
                argumentsCount--;
                break;
            }

            memset(arguments[argumentsCount], 0, 4096); // clear the argument

            i = 0;    // reset index
            buffer++; // skip space
            continue;
        }

        arguments[argumentsCount][i++] = *(buffer++); // copy byte by byte in the corresponding buffer
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
        else if (*arguments[1] == '/') // go to full path
        {
            memset(cwdBuffer, 0, 4096);                            // clear the buffer
            memcpy(cwdBuffer, arguments[1], strlen(arguments[1])); // copy the buffer
        }
        else // relative path
        {
            if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
                cwdBuffer[strlen(cwdBuffer)] = '/';

            sprintf(cwdBuffer, "%s%s", cwdBuffer, arguments[1]); // combine the current working directory buffer with the relative path
        }

        if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
            cwdBuffer[strlen(cwdBuffer)] = '/';

        uint64_t status = sys_vfs(SYS_VFS_DIRECTORY_EXISTS, (uint64_t)cwdBuffer, 0); // check if the directory exists

        if (!status)
        {
            printf("Couldn't find directory %s\n", arguments[1]);

            memset(cwdBuffer, 0, 4096); // clear cwd
            return;
        }

        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t)cwdBuffer, 0); // set the current working directory buffer
        return;
    }

    // append the extension if it doesn't exist
    if (strlen(arguments[0]) < strlen(".mx") || memcmp(arguments[0] + strlen(arguments[0]) - 3, ".mx", 3) != 0) // check for size and for last 3 bytes to match
        memcpy(arguments[0] + strlen(arguments[0]), ".mx", 4);                                                  // copy the extension (including the NULL)

    sprintf(cmdBuffer, "%s", arguments[0]); // copy the input in command buffer

    uint64_t fd = sys_open(arguments[0]);

    // try to append the path
    if (!fd)
    {
        sprintf(cmdBuffer, "%s%s", path, arguments[0]); // combine the path and the input

        uint64_t fd = sys_open(cmdBuffer); // check again for existence
        if (fd && strlen(cmdBuffer))
        {
            sys_close(fd); // close it if it exists
            goto execute;
        }

        printf("Couldn't find executable %s\n", arguments[0]);
        return;
    }
    else
        sys_close(fd); // close it if it exists

execute:
    uint64_t newPid;

    char *argv[MAX_ARGUMENTS];
    argumentsCount = min(MAX_ARGUMENTS - 1, argumentsCount); // clamp the value to the maximum

    for (int i = 0; i < argumentsCount; i++) // prepare the argument vector
        argv[i] = arguments[i + 1];

    sys_exec_packet_t p = {false, enviroment, cwdBuffer, argumentsCount, argv}; // prepare a packet
    sys_exec(cmdBuffer, &newPid, &p);                                           // send the kernel the packet

    uint64_t status;
    do
    {
        status = sys_pid(newPid, SYS_PID_STATUS, 0, 0); // get the status of the pid
        sys_yield();                                    // don't waste cpu time
    } while (status == 0);                              // wait for the pid to be stopped
}

int main(int argc, char **argv)
{
    puts("m Shell\n");

    // keyboard buffer
    char *kBuffer = malloc(4096);
    uint16_t kIdx = 0;
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
    sys_pid(pid, SYS_PID_GET_ENVIROMENT, (uint64_t)enviroment, 0); // get the enviroment

    if (argc >= 2)
        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t)argv[1], 0); // set the current working directory to the argument

    // allocate the arguments buffers
    for (int i = 0; i < MAX_ARGUMENTS; i++)
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

        // determine the path len
        uint16_t len;
        for (len = 0; path[len] != '|'; len++)
            ;

        path[len] = 0; // terminate path
    }

    // main loop
    while (1)
    {
        sys_pid(pid, SYS_PID_GET_CWD, (uint64_t)cwdBuffer, 0); // get the current working directory buffer
        memset(kBuffer, 0, 4096);                              // clear the buffer

        char chr;

        printf("%s m$ ", cwdBuffer); // print the prompt

        // read in the buffer
        do
        {
            chr = sys_input_keyboard(); // read a character off the keyboard buffer

            if (chr == '\b') // handle backspace
            {
                if (!kIdx) // don't overrun buffer
                    continue;

                kBuffer[--kIdx] = 0; // clear last character in buffer
                putchar('\b');       // remove last character
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