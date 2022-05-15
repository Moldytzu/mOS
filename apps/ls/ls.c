#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main()
{
    uint64_t pid, no;
    char *cwdBuffer, *dirBuffer;

    // current working directory buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cwdBuffer, 0);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    // directory listing buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&dirBuffer, 0);
    assert(dirBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET, &pid); // get the pid
    sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer
    sys_vfs(SYS_VFS_LIST_DIRECTORY, (uint64_t)cwdBuffer, (uint64_t)dirBuffer);

    puts("Listing ");
    puts(cwdBuffer);
    puts("\n");
    puts(dirBuffer);
    puts("\n");
}