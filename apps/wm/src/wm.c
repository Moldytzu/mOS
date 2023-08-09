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
        sys_input_mouse(&mouseX, &mouseY); // read mouse coordinates

        if (mouseX == oldX && mouseY == oldY) // update only on change
            continue;

        printf("x: %d, y: %d ", mouseX, mouseY);

        oldX = mouseX;
        oldY = mouseY;
    }
}