#include <stdlib.h>
#include <sys.h>

void exit(int status)
{
    sys_exit(status);
}
