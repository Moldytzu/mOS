#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    puts("m Init System\n"); // display a basic message

    int *a = (int *)0x1FF;
    *a = 1; // page fault

    while(1)
    {
        char chr;
        sys_input(SYS_INPUT_KEYBOARD,&chr); // read a character off the keyboard buffer
        
        if(chr) // if one exists then print it
            putchar(chr);
    }

    while(1); // the init system never returns
}