#include <ctype.h>
#include <stdio.h>

int toupper(int c)
{
    if (islower(c))
        return c - 'a' + 'A';
    return c;
}

int tolower(int c)
{
    if (isupper(c))
        return c - 'A' + 'a';
    return c;
}

int isxdigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

int islower(int c)
{
    return c >= 'a' && c <= 'z';
}

int isspace(int c)
{
    return c == ' ' || c == '\t';
}

int ispunct(int c)
{
    return (c >= '!' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}

int isprint(int c)
{
    return c >= ' ' && c <= '~';
}

int isgraph(int c)
{
    return isprint(c);
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int iscntrl(int c)
{
    return !isprint(c);
}

int isblank(int c)
{
    return isspace(c);
}

int isalpha(int c)
{
    return isupper(c) || islower(c);
}

int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}