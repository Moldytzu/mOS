#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

void test(const char *name, bool result)
{
    puts(name);
    puts("...");

    if(result)   
        puts("ok\n");
    else
        puts("fail\n");
}

int main(int argc, char **argv)
{
    puts("Testing libc...\n");

    // assert
    assert(1 == 1);
    test("assert", 1 == 1);
    
    // errno
    #ifdef EDOM 
    #define RESULT 1
    #else
    #define RESULT 0
    #endif

    test("errno", RESULT);

    #undef RESULT

    // stdlib
    test("abs", abs(-1) == 1 && abs(1) == 1);
    test("atoi", atoi("123") == 123);
    test("labs", labs(-1) == 1 && labs(1) == 1);
    test("atol", atol("123") == 123);
    test("malloc", malloc(16) != NULL);
    free(malloc(16));
    test("free", true);

    // stdio

    // string
    test("memcmp", memcmp("abc", "abc", 3) == 0);
    test("strlen", strlen("abc") == 3);
    test("strcmp", strcmp("abc", "abc") == 0);
}