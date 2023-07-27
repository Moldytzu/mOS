#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    puts("Writing garbage to NULL.\n");

    uint64_t *null = NULL;
    *null = 0xdeadbeef;
}