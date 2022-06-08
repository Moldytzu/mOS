#include <stdio.h>
#include <string.h>
#include <sys.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        puts("Usage: ");
        puts(argv[0]);
        puts(" <shutdown/reboot>\n");
        return 1;
    }

    if (strcmp(argv[1], "shutdown") != 0 && strcmp(argv[1], "reboot") != 0)
    {
        puts("Unknown operation: ");
        puts(argv[1]);
        putchar('\n');
        return 1;
    }

    sys_socket(SYS_SOCKET_WRITE, 1, (uint64_t)argv[1], strlen(argv[1])); // send the command to the init system

    while(1);
}