#include <stdlib.h>
#include <mos/sys.h>

int max(int a, int b)
{
    if (a > b)
        return a;

    return b;
}