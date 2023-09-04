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
    uint64_t targetX = atoi(argv[1]);
    uint64_t targetY = atoi(argv[2]);

    sys_display(SYS_DISPLAY_GET, (uint64_t)&oldX, (uint64_t)&oldY);   // get old resolution
    uint64_t status = sys_display(SYS_DISPLAY_SET, targetX, targetY); // set new resolution

    if (status) // failure
    {
        printf("Failed to set resolution. No framebuffer driver found.\n");
        abort();
    }
    else // success
    {
        sys_display(SYS_DISPLAY_GET, (uint64_t)&newX, (uint64_t)&newY); // get new resolution
        printf("Old resolution: %dx%d\nNew resolution: %dx%d\n", oldX, oldY, newX, newY);
    }

    return 0;
}