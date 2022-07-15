#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <shutdown/reboot>\n", argv[0]);
        abort();
    }

    if (strcmp(argv[1], "shutdown") != 0 && strcmp(argv[1], "reboot") != 0)
    {
        printf("Unknown operation: %s\n", argv[1]);
        abort();
    }

    sys_socket(SYS_SOCKET_WRITE, 1, (uint64_t)argv[1], strlen(argv[1])); // send the command to the init system

    while (1)
        ;
}