#include <sys.h>

extern int main();

void _mmain() // main function
{
    sys_exit(main()); // call the main function that returns a status code
}