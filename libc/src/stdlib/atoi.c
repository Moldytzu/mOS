#include <stdlib.h>
#include <sys.h>

int atoi(const char *str)
{
    int result = 0, i = 0;

    if(*str == '-') // skip sign
        i++;

    for(; str[i]; i++) // do the conversion
        result = result * 10 + str[i] - '0';

    if(*str == '-') // make the number negative
        result /= -1;

    return result;
}

long atol(const char *str)
{
    long result = 0;

    for(int i = 0; str[i]; i++) // do the conversion
        result = result * 10 + str[i] - '0';

    return result;
}