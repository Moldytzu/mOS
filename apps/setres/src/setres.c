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

    if (oldX == targetX && oldY == targetY)
    {
        printf("No resolution to change.\n");
        return 0;
    }

    if (status) // failure
    {
        printf("Failed to set resolution. No framebuffer driver found.\n");
        abort();
    }
    else // success
    {
        // wait for resolution change
        for (int i = 0; i < 50; i++)
        {
            sys_yield();
            sys_display(SYS_DISPLAY_GET, (uint64_t)&newX, (uint64_t)&newY); // get new resolution

            if (newX != oldX || newY != oldY) // resolution has changed
            {
                printf("Old resolution: %dx%d\nNew resolution: %dx%d\n", oldX, oldY, newX, newY);
                return 0;
            }
        }

        // it didn't change
        printf("Resolution change timed out.\n");
        abort();
    }

    return 0;
}