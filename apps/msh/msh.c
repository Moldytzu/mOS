#include <sys.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    puts("m Shell\n");

    char *kBuffer;
    uint16_t kIdx;
    sys_mem(SYS_MEM_ALLOCATE,(uint64_t)&kBuffer,0);

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
            {
                putchar(chr);
                kBuffer[kIdx++] = chr; // append the character
            }
        } while (chr != '\n');
        kIdx = 0;

        puts("You entered ");
        puts(kBuffer);
    }
}