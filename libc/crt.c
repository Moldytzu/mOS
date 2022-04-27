#include <sys.h>

extern int main();

void _mmain() // main function
{
    _syscall(SYS_EXIT, main(), 0, 0, 0, 0); // call the main function that returns an exit code
    while (1)
        ; // endless loop
}