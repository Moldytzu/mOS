#include <mos/sys.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    // simulate a crash
    uint64_t *ptr = NULL;
    *ptr = 0x69420;
}