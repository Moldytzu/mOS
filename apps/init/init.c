#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // set tty display mode
    sys_display(SYS_DISPLAY_CALL_SET, SYS_DISPLAY_TTY, 0);

    puts("m Init System is setting up your enviroment\n"); // display a welcome screen

    // set enviroment variables
    // create temp files
    // etc.

    // launch the shell
    puts("Launching msh from /init/msh.mx\n");

    uint64_t pid, status;
    sys_exec("/init/msh.mx", 0, &pid);

    do
    {
        sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
    } while (status == 0);                     // wait for the pid to be stopped

    puts("The shell stopped. Hanging.\n");

    while (1)
        ; // the init system never returns
}