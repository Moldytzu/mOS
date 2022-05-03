#include <stdlib.h>
#include <sys.h>

// wrappers of the system call
void abort()
{
    exit(EXIT_FAILURE);
}

void exit(int status)
{
    sys_exit(status);
}

int abs(int x)
{
    if (x < 0)
        x /= -1; // math trick to make a negative integer a positive one

    return x;
}

long int labs(long int x)
{
    if (x < 0)
        x /= -1; // math trick to make a negative integer a positive one

    return x;
}