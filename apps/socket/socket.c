#include <stdio.h>
#include <sys.h>

int main(int argc, char **argv)
{
    sys_socket(SYS_SOCKET_WRITE, 1, (uint64_t)"abc", 3);  // write abc to init's socket
}