#include <stdio.h>
#include <stdlib.h>
#include <mos/sys.h>
#include <string.h>

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage: %s <socket> <data>\n",argv[0]);
        abort();
    }

    sys_socket(SYS_SOCKET_WRITE, abs(atoi(argv[1])), (uint64_t)argv[2], strlen(argv[2]));  // write the data to the socket
}