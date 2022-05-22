#include <stdio.h>
#include <sys.h>
#include <assert.h>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        puts("Usage: ");
        puts(argv[0]);
        puts(" <file>\n");
        return 1;
    }

    uint64_t fd, size;
    sys_open(argv[1], &fd); // open the file
    assert(fd != 0);

    sys_vfs(SYS_VFS_FILE_SIZE, fd, (uint64_t)&size); // get the size
    assert(size != 0);

    // allocate the buffer
    void *buffer;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&buffer, 0);
    assert(buffer != NULL);

    sys_read(buffer, size, fd); // read the file

    puts(buffer); // print the buffer

    sys_close(fd); // close the file
}