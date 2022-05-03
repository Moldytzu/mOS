#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    puts("m Shell\n");

    uint64_t *ptr;
    sys_mem(SYS_MEM_ALLOCATE,(uint64_t)&ptr,0);
    *ptr = 0xFFFF; // set data

    // main loop
    while (1)
    {
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
}