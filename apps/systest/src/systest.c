#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TEST(x) printf("\nTESTING %s\n", x);

// convert to a string (base 16)
char to_hstringout[32];
const char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    memset(to_hstringout, 0, sizeof(to_hstringout)); // clear output

    for (int i = 0; i < 16; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];    // get each nibble

    strrev(to_hstringout); // reverse string

    // move the pointer until the first valid digit
    uint8_t offset = 0;
    for (; to_hstringout[offset] == '0'; offset++)
        ;

    return to_hstringout + offset; // return the string
}

int main(int argc, char **argv)
{
    printf("system call testing utility\n");

    TEST("display")
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);

        // can't test resolution set and get since we don't have any driver running
    }

    TEST("memory")
    {
        void *buffer = NULL;
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&buffer, 0);
        assert(buffer != NULL);

        printf("allocated %s\n", to_hstring((uint64_t)buffer)); // for some reason the %x format is broken in libc

        uint64_t used, available;
        sys_perf(SYS_PERF_GET_MEMORY, (uint64_t)&used, (uint64_t)&available);

        printf("used %d kB, available %d kB\n", used / 1024, available / 1024);

        assert(used > 0 && available > 0);
    }

    TEST("vfs")
    {
    }

    TEST("pid")
    {
        uint64_t pid = sys_pid_get();

        printf("we're pid %d\n", pid);

        const char env[4096], env2[4096];
        memset((void *)env, 0, 4096);
        memset((void *)env2, 0, 4096);
        memcpy((void *)env, "test env", strlen("test env")); // set an arbritary enviroment

        sys_pid(pid, SYS_PID_SET_ENVIROMENT, (void *)env);
        sys_pid(pid, SYS_PID_GET_ENVIROMENT, (void *)env2);

        printf("set enviroment to %s\n", env2);

        assert(env2[0] != '\0');

        const char cwd[512], cwd2[512];
        memset((void *)cwd, 0, 512);
        memset((void *)cwd2, 0, 512);
        memcpy((void *)cwd, "/init", strlen("/init")); // set an arbritary enviroment

        sys_pid(pid, SYS_PID_SET_CWD, (void *)cwd);
        sys_pid(pid, SYS_PID_GET_CWD, (void *)cwd2);

        printf("set working directory to %s\n", cwd2);

        assert(cwd2[0] != '\0');
    }

    printf("system passed\n");

    while (1)
        ;
}