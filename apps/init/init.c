#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    puts("m Init System\n");

    while(1)
    {
        char chr;
        sys_input(SYS_INPUT_KEYBOARD,&chr);
        
        if(chr)
            putchar(chr);
    }

    while(1); // the init system never returns
}