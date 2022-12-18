#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s <screen width> <screen height>\n", argv[0]);
        abort();
    }

    uint64_t oldX, oldY, newX, newY;
    sys_display(SYS_DISPLAY_GET, (uint64_t)&oldX, (uint64_t)&oldY); // get old resolution

    sys_display(SYS_DISPLAY_SET, atoi(argv[1]), atoi(argv[2])); // set new resolution

    for (int i = 0; i < 10; i++) // wait for the resolution to change
        sys_yield();

    sys_display(SYS_DISPLAY_GET, (uint64_t)&newX, (uint64_t)&newY); // get new resolution
    printf("Old resolution: %dx%d\nNew resolution: %dx%d\n", oldX, oldY, newX, newY);
}