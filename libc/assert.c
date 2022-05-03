#include <sys.h>
#include <stdio.h>

void assert(int expression)
{
    if(expression == 0)
    {
        puts("Assertion failed!\n");
        sys_exit(1);
    }
}