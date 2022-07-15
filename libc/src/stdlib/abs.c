#include <stdlib.h>
#include <sys.h>

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