#include <stdio.h>

void fprintf(FILE *restrict stream, const char *restrict format, ...)
{
    puts("fprintf stub\n");
}

int fgetc(FILE *stream)
{
    puts("fgetc stub\n");
    return 0;
}
char *fgets(char *restrict s, int n, FILE *restrict stream)
{
    puts("fgets stub\n");
    return 0;
}
int fputc(int c, FILE *stream)
{
    puts("fputc stub\n");
    return 0;
}
int fputs(const char *restrict s, FILE *restrict stream)
{
    puts("fputs stub\n");
    return 0;
}
int remove(const char *filename)
{
    puts("remove stub\n");
    return 0;
}
int rename(const char *old, const char *new)
{
    puts("rename stub\n");
    return 0;
}
FILE *tmpfile(void)
{
    puts("tmpfile stub\n");
    return 0;
}
char *tmpnam(char *s)
{
    puts("tmpnam stub\n");
    return 0;
}
int fclose(FILE *stream)
{
    puts("fclose stub\n");
    return 0;
}
int fflush(FILE *stream)
{
    puts("fflush stub\n");
    return 0;
}
FILE *fopen(const char *restrict filename, const char *restrict mode)
{
    puts("fopen stub\n");
    return 0;
}
FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream)
{
    puts("freopen stub\n");
    return 0;
}

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream)
{
    puts("fread stub\n");
    return 0;
}
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream)
{
    puts("fwrite stub\n");
    return 0;
}

long int ftell(FILE *stream)
{
    puts("ftell stub\n");
    return 0;
}

int fseek(FILE *stream, long int offset, int whence)
{
    puts("fseek stub\n");
    return 0;
}

int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg)
{
    puts("vfprintf stub\n");
    return 0;
}