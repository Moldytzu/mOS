#include <stdlib.h>
#include <mos/sys.h>

int min(int a, int b)
{
    if (a > b)
        return b;

    return a;
}

