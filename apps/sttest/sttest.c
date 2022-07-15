#include <stdio.h>
#include <sys.h>
#include <assert.h>
#include <stdlib.h>

#define TESTS 17 /*max tests till system crash, current target is 20*/

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
    cwdBuffer = malloc(512);
    assert(cwdBuffer != NULL); // assert that the buffer is valid

    sys_pid(0, SYS_PID_GET, &pid);                        // get the pid
    sys_pid(pid, SYS_PID_GET_CWD, (uint64_t *)cwdBuffer); // get the current working directory buffer

    for (int i = 0; i < TESTS; i++)
        test("/init/blank.mx"); // does nothing

    sys_pid(0, SYS_PID_SLEEP, (uint64_t *)(TESTS * 50)); // wait for all the pids to stop
    puts("The system survived!\n");
}