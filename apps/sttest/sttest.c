#include <stdio.h>
#include <sys.h>
#include <assert.h>

uint64_t pid;
char *cwdBuffer;

void test(const char *path)
{
    uint64_t newPid, status;
    struct sys_exec_packet p = {0, "|", cwdBuffer, 0, 0};
    sys_exec(path, &newPid, &p); // execute ls
}

int main(int argc, char **argv)
{
    // current working directory buffer
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&cwdBuffer, 0);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET, &pid);                        // get the pid
    sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer

    for (int i = 0; i < 25; i++)
        test("/init/ls.mx"); // list directory

    sys_pid(0, SYS_PID_SLEEP, (uint64_t *)5000); // wait for all the pids to stop
    puts("The system survived!\n");
}