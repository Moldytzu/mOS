#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // set tty display mode
    sys_display(SYS_DISPLAY_CALL_SET,SYS_DISPLAY_TTY,0);

    puts("m Init System\n"); // display a basic message

    while(1)
    {
        char chr;
        sys_input(SYS_INPUT_KEYBOARD,&chr); // read a character off the keyboard buffer
        
        if(chr) // if one exists then print it
            putchar(chr);
    }

    while(1); // the init system never returns
}