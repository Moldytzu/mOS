#include <stdio.h>
#include <sys.h>

int main()
{
    uint64_t fd;
    sys_open("/init/test.file", &fd);

    if (!fd)
    {
        puts("Failed to open the file.\n");
        return 0;
    }

    sys_close(fd);
}