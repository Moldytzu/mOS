#include <stdio.h>
#include <stdlib.h>
#include <sys.h>
#include <string.h>

void puts(const char *str)
{
    sys_write((void*)str,strlen(str),1);
}