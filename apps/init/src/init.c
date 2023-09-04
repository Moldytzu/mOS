#include <mos/sys.h>
#include <mos/drv.h>
#include <libcfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define NANOS_TO_MILIS 1000000
#define SOCKET_SIZE 4096
#define DRIVERS

uint64_t maxPowerTimeout = 0;
uint64_t powerConfirmations = 0;
uint64_t sockID = 0;
void *sockBuffer = NULL;
bool verbose = true;
bool safe = false;
char *shell = "/init/msh.mx";
config_t cfg;

uint64_t uptimeMilis()
{
    return sys_time_uptime_nanos() / NANOS_TO_MILIS;
}

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
    maxPowerTimeout = cfgUint(&cfg, "POWER_TIMEOUT");
    powerConfirmations = cfgUint(&cfg, "POWER_CONFIRMATIONS");

    if (!maxPowerTimeout)
        maxPowerTimeout = 1000;

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

    if (sys_display(SYS_DISPLAY_SET, screenX, screenY) /*return 1 on failure*/)
        printf("Failed to set screen resolution to %ux%u\n", screenX, screenY);
}

// linear search the toSearch string in the por
char *search(char *str, char *toSearch)
{
    if (strlen(toSearch) > strlen(str)) // don't bother if searched string is bigger than the actual string
        return NULL;

    size_t maxLen = strlen(str) - strlen(toSearch); // this can be zero when strlen(toSearch) == strlen(str)
    for (int i = 0; i <= maxLen; i++)               // thus we have to count up to maxLen (including it!)
    {
        if (memcmp(str, toSearch, strlen(toSearch)) == 0) // do memory comparison at the offset
            return str;                                   // return the address

        str++;
    }

    return NULL; // fail
}

uint64_t powerTimestamp;
uint8_t powerCount;
void handleSocket()
{
    sys_socket(SYS_SOCKET_READ, sockID, (uint64_t)sockBuffer, SOCKET_SIZE); // read the whole socket
    if (!*(char *)sockBuffer)                                               // if empty give up
        return;

    if (search(sockBuffer, "crash "))
    {
        printf("%s has crashed!\n", sockBuffer + 6 /*skip "crash "*/);
    }

    if (search(sockBuffer, "shutdown")) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Shutdowning...");
        sys_power(SYS_POWER_SHUTDOWN, 0, 0);
    }

    if (search(sockBuffer, "reboot")) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Rebooting...");
        sys_power(SYS_POWER_REBOOT, 0, 0);
    }

    if (search(sockBuffer, "acpi_power")) // power button
    {
        uint64_t difference = uptimeMilis() - powerTimestamp;

        if ((powerCount == powerConfirmations && difference < maxPowerTimeout) || powerConfirmations == 0)
        {
            sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
            puts("\n\n\n Shutdowning...");
            sys_power(SYS_POWER_SHUTDOWN, 0, 0);
        }
        else if (difference >= maxPowerTimeout) // timeout
        {
            powerCount = 0;
        }

        if (powerCount == 0) // if it's reset or first time print the message
            printf("\nPress %d times in maximum %d miliseconds between presses to confirm shutdown\n", powerConfirmations, maxPowerTimeout);

        powerCount++;
        powerTimestamp = uptimeMilis();
    }

    memset(sockBuffer, 0, SOCKET_SIZE); // clear the socket buffer

    sys_yield();
}

int main(int argc, char **argv)
{
    uint64_t kernelStartupTime = uptimeMilis();

    // ensure that the pid is 1
    if (sys_pid_get() != 1)
    {
        puts("The init system has to be launched as PID 1!\n");
        sys_exit(1);
    }

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

    uint64_t userspaceStartupTime = uptimeMilis();

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