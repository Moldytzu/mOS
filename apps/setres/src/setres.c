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

    sys_display(SYS_DISPLAY_GET, (uint64_t)&oldX, (uint64_t)&oldY); // get old resolution
    sys_display(SYS_DISPLAY_SET, targetX, targetY);                 // set new resolution

    int i = 0;
    for (; i < 50; i++) // wait for the resolution to change
    {
        uint64_t currentX, currentY;
        sys_display(SYS_DISPLAY_GET, (uint64_t)&currentX, (uint64_t)&currentY); // get current resolution

        if (currentX == targetX && currentY == targetY) // if resolution matches then we can break
            break;

        sys_yield();
    }

    if (i == 20)
    {
        printf("Resolution change timed out. (no framebuffer driver running?)\n");
        abort();
    }

    sys_display(SYS_DISPLAY_GET, (uint64_t)&newX, (uint64_t)&newY); // get new resolution
    printf("Old resolution: %dx%d\nNew resolution: %dx%d\n", oldX, oldY, newX, newY);
}