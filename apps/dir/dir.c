#include <sys.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    uint64_t pid;
    char *cwdBuffer;

    // current working directory buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cwdBuffer, 0);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET, &pid);                        // get the pid
    sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer

    uint64_t newPid;
    struct sys_exec_packet p = {0, "|", cwdBuffer, 0, 0};
    sys_exec("/init/ls.mx", &newPid, &p);
}