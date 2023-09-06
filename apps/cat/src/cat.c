#include <stdio.h>
#include <mos/sys.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    uint64_t fd, size;
    fd = sys_open(argv[1]); // open the file
    if (!fd)
    {
        printf("%s: Failed to open %s\n", argv[0], argv[1]);
        return 1;
    }

    size = sys_vfs(SYS_VFS_FILE_SIZE, fd, 0); // get the size
    if (!size)
    {
        printf("%s: Attempt to open empty file %s\n", argv[0]);
        sys_close(fd);
        return 1;
    }

    // allocate the buffer
    void *buffer = malloc(size);
    assert(buffer != NULL);

    sys_read(buffer, size, fd); // read the file

    puts(buffer); // print the buffer

    sys_close(fd); // close the file
}