#include <stdio.h>
#include <stdlib.h>
#include <mos/sys.h>
#include <string.h>

void putchar(int c)
{
    sys_write(&c,1,1);
}