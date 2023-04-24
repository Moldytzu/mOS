#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define SOCKET_SIZE 4096
#define DRIVERS

uint64_t sockID = 0;
void *sockBuffer = NULL;
bool verbose = true;
bool safe = false;
char *shell = "/init/msh.mx";
char *cfg;
char *drivers[512]; // 512 max drivers should be enough for now
uint8_t driverIdx = 0;

void eventLoop()
{
    sys_socket(SYS_SOCKET_READ, sockID, (uint64_t)sockBuffer, SOCKET_SIZE); // read the whole socket
    if (!*(char *)sockBuffer)                                               // if empty give up
        return;

    if (memcmp(sockBuffer, "crash ", 6) == 0)
    {
        printf("%s has crashed!\n", sockBuffer + 6 /*skip "crash "*/);
    }

    if (strcmp(sockBuffer, "shutdown") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Shutdowning...");
        sys_power(SYS_POWER_SHUTDOWN, 0, 0);
    }

    if (strcmp(sockBuffer, "reboot") == 0) // shutdown command
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0); // set mode to tty
        puts("\n\n\n Rebooting...");
        sys_power(SYS_POWER_REBOOT, 0, 0);
    }

    memset(sockBuffer, 0, SOCKET_SIZE); // clear the socket buffer

    sys_yield();
}

void parseCFG()
{
    // buffer for config file
    uint64_t fd, size;
    cfg = malloc(4096);
    assert(cfg != NULL);

    fd = sys_open("/init/init.cfg"); // open the file
    assert(fd != 0);

    sys_vfs(SYS_VFS_FILE_SIZE, fd, (uint64_t)&size); // get the size
    assert(size != 0);

    memset(cfg, 0, 4096);               // clear the buffer
    sys_read(cfg, min(size, 4096), fd); // read the file

    memset(drivers, 0, sizeof(drivers)); // clear the driver addresses

    shell = "/init/msh.sh"; // set a default hard-coded shell location

    // todo: make this code easier to read
    for (int i = 0; i < 4096; i++)
    {
        // check for verbose flag
        if (memcmp(cfg + i, "VERBOSE = ", strlen("VERBOSE = ")) == 0)
        {
            verbose = cfg[i + strlen("VERBOSE = ")] == '1';
            i += strlen("VERBOSE = ");
        }

        // check for safe flag
        if (memcmp(cfg + i, "SAFE = ", strlen("SAFE = ")) == 0)
        {
            safe = cfg[i + strlen("SAFE = ")] == '1';
            i += strlen("SAFE = ");
        }

        // check for driver path
        if (memcmp(cfg + i, "DRIVER \"", strlen("DRIVER \"")) == 0)
        {
            // calculate length of the driver path
            size_t len = 0;
            for (; cfg[i + strlen("DRIVER \"") + len] != '\"'; len++)
                ;

            // terminate the string
            cfg[i + strlen("DRIVER \"") + len] = '\0';

            // set the pointer
            drivers[driverIdx++] = cfg + strlen("DRIVER \"") + i;

            i += strlen("DRIVER \"") + len;
        }

        // check for driver path
        if (memcmp(cfg + i, "SHELL \"", strlen("SHELL \"")) == 0)
        {
            // calculate length of the driver path
            size_t len = 0;
            for (; cfg[i + strlen("SHELL \"") + len] != '\"'; len++)
                ;

            // terminate the string
            cfg[i + strlen("SHELL \"") + len] = '\0';

            // set the pointer
            shell = cfg + strlen("SHELL \"") + i;

            i += strlen("SHELL \"") + len;
        }
    }

    if (safe)
        verbose = true; // force verbose to true if we're in safe mode

    if (verbose)
        puts("m Init System is setting up your enviroment\n"); // display a welcome screen

    // start the drivers if safe mode isn't enabled
    if (safe)
        return;

    for (int i = 0; i < driverIdx; i++)
    {
        if (!drivers[i])
            continue;

        if (verbose)
            printf("Starting driver from %s\n", drivers[i]); // for some reason it shows null

#ifdef DRIVERS
        sys_driver(SYS_DRIVER_START, (uint64_t)(drivers[i]), 0, 0);
#endif
    }
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
    sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);

    puts("Attention! mOS is an FOSS operating system (licensed under the MIT license) that doesn't gurantee any compatibility with existing software nor stability as it is in a pre-alpha stage. We strongly advise to test this OS only in virtual machines.\n"); // display a disclaimer

    // parse the config
    parseCFG();

    if (!safe)
    {
        sys_display(SYS_DISPLAY_SET, 1024, 768); // set screen resolution to 1024x768
        sys_yield();

        // wait for the change to happen
        for (int i = 0; i < 5; i++)
        {
            uint64_t width, height;
            sys_display(SYS_DISPLAY_GET, (uint64_t)&width, (uint64_t)&height); // get old resolution

            if (width == 1024 && height == 768)
                break;

            sys_yield();
        }
    }

    // create a socket for ipc
    sys_socket(SYS_SOCKET_CREATE, (uint64_t)&sockID, 0, 0);

    assert(sockID != 0); // assert that the socket is valid

    sockBuffer = malloc(SOCKET_SIZE);
    assert(sockBuffer != NULL); // assert that the socket buffer is valid

    // start a shell
    const char *enviroment = "PATH=/init/|"; // the basic enviroment

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
            eventLoop(); // run the event loop

            sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
        } while (status == 0);                     // wait for the pid to be stopped

        if (verbose)
            puts("The shell has stopped. Relaunching it.\n");
    }

    while (1) // the init system never returns
        sys_yield();
}