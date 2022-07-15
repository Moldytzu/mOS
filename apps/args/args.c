#include <stdio.h>

int main(int argc, char **argv)
{
    puts("Arguments: \n");
    for(int i = 0; i < argc; i++)
    {
        puts(argv[i]);
        putchar(' ');
    }
    putchar('\n');
}