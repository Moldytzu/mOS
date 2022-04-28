#include <sys.h>

int main()
{
    const char msg[] = "m Init System for mOS\n";
    sys_write((void*)msg, sizeof(msg) - 1, 1); // write the message on stdout
    return 1024;
}