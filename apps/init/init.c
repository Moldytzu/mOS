#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // set tty display mode
    sys_display(SYS_DISPLAY_CALL_SET, SYS_DISPLAY_TTY, 0);

    puts("m Init System\n"); // display a basic message

    uint64_t pid, status;
    sys_exec("/init/hello.mx", 0, &pid); // exec hello world on the same terminal
    
    do
    {
        sys_pid(pid, SYS_PID_STATUS, &status); // get the status of the pid
    } while(status == 0); // wait for the pid to be stopped

    puts("The app stopped!\n");

    while (1)
    {
        char chr;
        sys_input(SYS_INPUT_KEYBOARD, &chr); // read a character off the keyboard buffer

        if (chr) // if one exists then print it
            putchar(chr);
    }

    while (1)
        ; // the init system never returns
}