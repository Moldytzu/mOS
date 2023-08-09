#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    puts("Starting window manager.\n");

    // event loop
    uint16_t oldX, oldY;
    while (true)
    {
        uint16_t mouseX, mouseY;
        _syscall(SYS_INPUT, SYS_INPUT_MOUSE, (uint64_t)&mouseX, (uint64_t)&mouseY, 0, 0);

        if (mouseX == oldX && mouseY == oldY)
            continue;

        printf("x: %d, y: %d ", mouseX, mouseY);

        oldX = mouseX;
        oldY = mouseY;
    }
}