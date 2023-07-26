#include <stdlib.h>
#include <mos/sys.h>

void exit(int status)
{
    sys_exit(status);
}
