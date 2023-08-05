#include <mos/sys.h>
#include <mos/drv.h>
#include <libcfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define SOCKET_SIZE 4096
#define DRIVERS

uint64_t sockID = 0;
void *sockBuffer = NULL;
bool verbose = true;
bool safe = false;
char *shell = "/init/msh.mx";
config_t cfg;

void parseCFG()
{
    // display a welcome message
    puts("m Init System is setting up your enviroment\n");

    // buffer for config file
    uint64_t fd, size;
    void *buffer = malloc(4096);
    assert(buffer != NULL);

    fd = sys_open("/init/init.cfg"); // open the file
    assert(fd != 0);

    size = sys_vfs(SYS_VFS_FILE_SIZE, fd, 0); // get the size
    assert(size != 0);

    sys_read(buffer, min(size, 4096), fd); // read the file

    cfg = cfgCreate(buffer); // generate a new config context

    // parse the config
    safe = cfgBool(&cfg, "SAFE");
    verbose = cfgBool(&cfg, "VERBOSE") | safe; // verbose mode is forced on by safe
    shell = cfgStr(&cfg, "SHELL");

    uint32_t screenX = cfgUint(&cfg, "SCREEN_WIDTH");
    uint32_t screenY = cfgUint(&cfg, "SCREEN_HEIGHT");

    if (!screenX | !screenY) // invalid resolution
    {
        // set a fail-safe resolution
        screenX = 640;
        screenY = 480;
    }

    if (safe) // safe mode doesn't initialise drivers
        return;

    // start drivers set in config
    char *drivers = cfgStr(&cfg, "DRIVERS");
    uint16_t driversLen = strlen(drivers);

    // the drivers string is in the form "driver;driver2;driver3 etc etc"
    char *currentDriver = drivers;
    for (int i = 0; i < driversLen; i++)
    {
        if (drivers[i] == ' ') // ignore whitespace (improves readability)
        {
            currentDriver++;
            continue;
        }

        if (drivers[i] == ';') // start each driver after the separator
        {
            drivers[i] = '\0';               // terminate string
            sys_driver_start(currentDriver); // start the driver

            if (verbose)
                printf("Started driver %s\n", currentDriver);

            currentDriver = drivers + i + 1; // point to the new driver
        }
    }

    if (verbose)
        printf("Setting screen resolution to %ux%u\n", screenX, screenY);

    sys_display(SYS_DISPLAY_SET, screenX, screenY);
}

void handleSocket()
{
    sys_socket(SYS_SOCKET_READ, sockID, (uint64_t)sockBuffer, SOCKET_SIZE); // read the whole socket
    if (!*(char *)sockBuffer)                                               // if empty give up
        return;

    if (memcmp(sockBuffer, "crash ", 6) == 0)
    {
        printf("%s has crashed!\n", sockBuffer + 6 /*skip "crash "*/);
    }
    else if (strcmp(sockBuffer, "shutdown") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Shutdowning...");
        sys_power(SYS_POWER_SHUTDOWN, 0, 0);
    }
    else if (strcmp(sockBuffer, "reboot") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Rebooting...");
        sys_power(SYS_POWER_REBOOT, 0, 0);
    }
    else
    {
        printf("Unknown socket packet: %s\n", sockBuffer);
    }

    memset(sockBuffer, 0, SOCKET_SIZE); // clear the socket buffer

    sys_yield();
}

int main(int argc, char **argv)
{
    // ensure that the pid is 1
    if (sys_pid_get() != 1)
    {
        puts("The init system has to be launched as PID 1!\n");
        sys_exit(1);
    }

    uint64_t kernelStartupTime = sys_time_uptime_nanos() / 1000000;

    // set tty display mode
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);

    // parse the config
    parseCFG();

    // create a socket for ipc
    sockID = sys_socket(SYS_SOCKET_CREATE, 0, 0, 0);

    assert(sockID != 0); // assert that the socket is valid

    sockBuffer = malloc(SOCKET_SIZE);
    assert(sockBuffer != NULL); // assert that the socket buffer is valid

    // start a shell
    const char *enviroment = "PATH=/init/|"; // the basic enviroment

    uint64_t userspaceStartupTime = sys_time_uptime_nanos() / 1000000;

    if (verbose)
        printf("Startup finished in %dms (kernel) + %dms (userspace) = %dms\n", kernelStartupTime, userspaceStartupTime - kernelStartupTime, userspaceStartupTime);

    while (1)
    {
        // launch the shell
        if (verbose)
            printf("Launching a shell from %s\n", shell);

        uint64_t pid, status;
        sys_exec_packet_t p = {0, enviroment, "/init/", 0, 0};
        sys_exec(shell, &pid, &p);

        do
        {
            handleSocket(); // handle the socket

            status = sys_pid(pid, SYS_PID_STATUS, 0, 0); // get the status of the pid
        } while (status == 0);                           // wait for the pid to be stopped

        if (verbose)
            puts("The shell has stopped. Relaunching it.\n");
    }

    printf("Init system execution jumped out of event loop. Probably a bug!");

    while (1)
        ;
}