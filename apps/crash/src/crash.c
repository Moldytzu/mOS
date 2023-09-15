#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    puts("Crash\nPress 1 - Page Fault\nPress 2 - Divide By Zero\n");

    char c;
    do
    {
        c = sys_input_keyboard();
    } while (!c || c <= '0' || c >= '3');

    switch (c)
    {
    case '1':
        puts("Writing garbage to NULL.\n");

        uint64_t *null = NULL;
        *null = 0xdeadbeef;
        break;
    case '2':
        puts("Dividing 5 by zero\n");

        int a = 0;
        int b = 5;
        volatile int c = b / a;
        break;
    }
}