#include <stdio.h>
#include <stdlib.h>
#include <sys.h>
#include <string.h>

void puts(const char *str)
{
    sys_write((void*)str,strlen(str),1);
}

void putchar(int c)
{
    sys_write(&c,1,1);
}