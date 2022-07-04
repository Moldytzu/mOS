#include <sys.h>

int main(int argc, char **argv)
{
    sys_pid(0, SYS_PID_SLEEP, (uint64_t *)1000); // wait a second
}