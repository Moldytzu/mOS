#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    char *cwdBuffer, *dirBuffer;

    // current working directory buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cwdBuffer, 0);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    // directory listing buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&dirBuffer, 0);
    assert(dirBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer);                        // get the current working directory buffer
    sys_vfs(SYS_VFS_LIST_DIRECTORY, (uint64_t)cwdBuffer, (uint64_t)dirBuffer); // list the directory

    puts(dirBuffer); // print the result
    puts("\n");
}