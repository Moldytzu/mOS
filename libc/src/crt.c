#include <mos/sys.h>

extern int main(int argc, char **argv);

void _mmain(int argc, char **argv) // main function
{
    sys_exit(main(argc, argv)); // call the main function that returns a status code
}