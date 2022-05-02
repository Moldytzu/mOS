#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    puts("m Shell\n");

    // main loop

        char chr;

        puts("m$ ");

        // read the buffer
        do
        {
            sys_input(SYS_INPUT_KEYBOARD, &chr); // read a character off the keyboard buffer
            if(chr)
                putchar(chr);
        } while (chr != '\n');
    
}