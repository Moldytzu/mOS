#include <stdio.h>

int main(int argc, char **argv)
{
    puts("Listing arguments\n");
    for(int i = 0; i < argc; i++)
    {
        puts(argv[i]);
        putchar('\n');
    }
}