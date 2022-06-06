#include <stdio.h>
#include <stdlib.h>
#include <sys.h>
#include <string.h>

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        puts("Usage: ");
        puts(argv[0]);
        puts(" <socket> <data>\n");
        abort();
    }

    sys_socket(SYS_SOCKET_WRITE, abs(atoi(argv[1])), (uint64_t)argv[2], strlen(argv[2]));  // write the data to the socket
}