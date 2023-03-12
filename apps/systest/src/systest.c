#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TEST(x)

// convert to a string (base 16)
char to_hstringout[32];
const char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    memset(to_hstringout, 0, sizeof(to_hstringout)); // clear output

    for (int i = 0; i < 16; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];    // get each nibble

    strrev(to_hstringout); // reverse string

    // move the pointer until the first valid digit
    uint8_t offset = 0;
    for (; to_hstringout[offset] == '0'; offset++)
        ;

    return to_hstringout + offset; // return the string
}

int main(int argc, char **argv)
{
    // assert(argc != 0);

    TEST("display")
    {
        sys_display(SYS_DISPLAY_MODE, SYS_DISPLAY_TTY, 0);

        // can't test resolution set and get since we don't have any driver running
    }

    TEST("io")
    {
        printf("system call testing utility\n");
    }

    TEST("memory")
    {
        void *buffer = NULL;
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&buffer, 0);
        assert(buffer != NULL);

        printf("allocated %s\n", to_hstring((uint64_t)buffer)); // for some reason the %x format is broken in libc
    }

    TEST("vfs")
    {
    }

    printf("system passed\n");

    while (1)
        ;
}