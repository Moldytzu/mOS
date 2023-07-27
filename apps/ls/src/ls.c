#include <mos/sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    char *cwdBuffer = malloc(4096), *dirBuffer = malloc(4096);

    assert(cwdBuffer != NULL && dirBuffer != NULL); // assert that the buffers are valid

    sys_pid(0, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer

    if (argc > 1) // list from argument
        sys_vfs(SYS_VFS_LIST_DIRECTORY, (uint64_t)argv[1], (uint64_t)dirBuffer);
    else // list from cwd
        sys_vfs(SYS_VFS_LIST_DIRECTORY, (uint64_t)cwdBuffer, (uint64_t)dirBuffer);

    printf("%s\n", dirBuffer); // print the listing
}