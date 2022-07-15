#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SOCKET_SIZE (4096 - 24)

uint64_t sockID = 0;
void *sockBuffer = NULL;

void eventLoop()
{
    sys_socket(SYS_SOCKET_READ, sockID, (uint64_t)sockBuffer, SOCKET_SIZE); // read the whole socket
    if (!*(char *)sockBuffer)                                               // if empty give up
        return;

    if (memcmp(sockBuffer, "crash ", 6) == 0)
    {
        puts(sockBuffer + 6);
        puts(" has crashed!\n");
    }

    if (strcmp(sockBuffer, "shutdown") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_CALL_SET, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Shutdowning...");
        sys_power(SYS_POWER_SHUTDOWN, 0, 0);
    }

    if (strcmp(sockBuffer, "reboot") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_CALL_SET, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Rebooting...");
        sys_power(SYS_POWER_REBOOT, 0, 0);
    }

    memset(sockBuffer, 0, SOCKET_SIZE); // clear the socket buffer
}

int main(int argc, char **argv)
{
    // ensure that the pid is 1
    uint64_t initPID;
    sys_pid(0, SYS_PID_GET, &initPID);

    if (initPID != 1)
    {
        puts("The init system has to be launched as PID 1!\n");
        sys_exit(1);
    }

    // set tty display mode
    sys_display(SYS_DISPLAY_CALL_SET, SYS_DISPLAY_TTY, 0);

    puts("m Init System is setting up your enviroment\n"); // display a welcome screen

    // create a socket for ipc
    sys_socket(SYS_SOCKET_CREATE, (uint64_t)&sockID, 0, 0);

    assert(sockID != 0); // assert that the socket is valid

    sockBuffer = malloc(SOCKET_SIZE);
    assert(sockBuffer != NULL); // assert that the socket buffer is valid

    const char *enviroment = "PATH=/init/|"; // the basic enviroment

    while (1)
    {
        // launch the shell
        puts("Launching msh from /init/msh.mx\n");

        uint64_t pid, status;
        struct sys_exec_packet p = {0, enviroment, "/init/", 0, 0};
        sys_exec("/init/msh.mx", &pid, &p);

        do
        {
            eventLoop(); // run the event loop

            sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
        } while (status == 0);                     // wait for the pid to be stopped

        puts("The shell stopped. Relaunching it.\n");
    }
    while (1)
        ; // the init system never returns
}