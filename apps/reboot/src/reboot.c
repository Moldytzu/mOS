#include <mos/sys.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    sys_socket(SYS_SOCKET_WRITE, 1, (uint64_t) "reboot", strlen("reboot")); // write "reboot" in init's socket (id 1)
}