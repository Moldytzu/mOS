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

    // clear the arguments buffer
    for(int i = 0; i < 32; i++)
        memset(arguments[i],0,4096);

    int i = 0;
    argumentsCount = 0;
    // split the buffer
    while(*buffer)
    {
        if(*buffer == ' ') // split
        {
            argumentsCount++; // switch the buffer

            if(argumentsCount > 32)
                break;

            i = 0; // reset index
            buffer++; // skip character
            continue;
        }
        arguments[argumentsCount][i++] = *(buffer++);
    }

    // display the arguments
    for(int i = 0; i <= argumentsCount; i++)
    {
        puts(arguments[i]);
        putchar(' ');
    }
    putchar('\n');

    if (strcmp(arguments[0], "exit") == 0) // exit command
        exit(EXIT_SUCCESS);

    if (memcmp(arguments[0], "cd ", 3) == 0) // change directory command
    {
        arguments[0] += 3; // skip "cd "

        if (strcmp(arguments[0], "..") == 0) // go back a folder
        {
            for (int i = strlen(cwdBuffer) - 2; cwdBuffer[i] != '/'; cwdBuffer[i--] = '\0')
                ; // step back to last delimiter
        }
        else if (*arguments[0] == '/') // full path
        {
            memset(cwdBuffer, 0, 4096);                // clear the buffer
            memcpy(cwdBuffer, arguments[0], strlen(arguments[0])); // copy the buffer
        }
        else
        {
            if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
                cwdBuffer[strlen(cwdBuffer)] = '/';

            memcpy(cwdBuffer + strlen(cwdBuffer), arguments[0], strlen(arguments[0])); // copy the buffer
        }

        if (cwdBuffer[strlen(cwdBuffer) - 1] != '/') // append the delimiter if it doesn't exist
            cwdBuffer[strlen(cwdBuffer)] = '/';

        uint64_t status;
        sys_vfs(SYS_VFS_DIRECTORY_EXISTS, (uint64_t)cwdBuffer, (uint64_t)&status); // check if the directory exists

        if (!status)
        {
            puts("Couldn't find directory ");
            puts(arguments[0]);
            putchar('\n');

            // restore the cwd
            sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer
            return;
        }

        sys_pid(pid, SYS_PID_SET_CWD, (uint64_t *)cwdBuffer); // set the current working directory buffer
        return;
    }

    if (memcmp(arguments[0], "./", 2) == 0) // this folder prefix
    {
        arguments[0] += 2;    // skip "./"
        goto appendCWD; // append the cwd and execute
    }

    uint16_t bufOffset = pathLen;

    // don't append anything if we specify the full path
    if (*arguments[0] == '/')
    {
        bufOffset = 0;
        goto inputContinue;
    }

    // don't append the path if we already specify it
    if (strlen(arguments[0]) <= pathLen)
        goto inputContinue;

    if (memcmp(arguments[0], path, pathLen) == 0)
        bufOffset = 0;

inputContinue:
    memset((void *)cmdBuffer, 0, 4096);                              // clear the buffer
    memcpy((void *)(cmdBuffer + bufOffset), arguments[0], strlen(arguments[0])); // copy the input
    if (bufOffset)
        memcpy((void *)cmdBuffer, path, pathLen); // copy the path

    // append the extension if it doesn't exist
    if (memcmp(cmdBuffer + strlen(cmdBuffer) - 3, ".mx", 3) != 0)
        memcpy((void *)(cmdBuffer + strlen(cmdBuffer)), ".mx", 3); // copy the extension

    uint64_t status;
    sys_vfs(SYS_VFS_FILE_EXISTS, (uint64_t)cmdBuffer, (uint64_t)&status); // check if file exists

    if (!status)
    {
    appendCWD:
        // trying to append the cwd
        memset((void *)cmdBuffer, 0, 4096); // clear the buffer
        bufOffset = strlen(cwdBuffer);
        memcpy((void *)(cmdBuffer + bufOffset), buffer, strlen(arguments[0])); // copy the input
        memcpy((void *)cmdBuffer, cwdBuffer, bufOffset);                 // copy the path

        sys_vfs(SYS_VFS_FILE_EXISTS, (uint64_t)cmdBuffer, (uint64_t)&status); // check if file exists
        if (status)
            goto execute;

        puts("Couldn't find executable ");
        puts(arguments[0]);
        putchar('\n');
        return;
    }

execute:
    uint64_t newPid;
    char *argv[] = {"abc"};
    struct sys_exec_packet p = {0, enviroment, cwdBuffer, 1, argv};
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