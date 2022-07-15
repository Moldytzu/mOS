#include <sys.h>
#include <stdio.h>

void __assert(const char *str, const char *file)
{
    puts(str);
    puts(" failed! (");
    puts(file);
    puts(")\n");
    sys_exit(1);
}