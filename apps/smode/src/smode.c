#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: %s <display mode>\n", argv[0]);
        abort();
    }

    sys_display(SYS_DISPLAY_MODE, atoi(argv[1]), 0);
}