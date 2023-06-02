#pragma once
#include <stdarg.h>

// page 274 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

#define stdout ((void *)0)
#define stderr ((void *)1)
#define stdin ((void *)2)

#define NULL ((void *)0)

#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 1

typedef unsigned long size_t;

typedef struct
{
    char stub;
} FILE;

// alias the underscore names of the printf standalone library
#define printf_ printf
#define sprintf_ sprintf
#define vsprintf_ vsprintf
#define snprintf_ snprintf
#define vsnprintf_ vsnprintf
#define vprintf_ vprintf

// printf
int printf_(const char *format, ...);
int vprintf_(const char *format, va_list arg);
int sprintf_(char *s, const char *format, ...);
int vsprintf_(char *s, const char *format, va_list arg);
int snprintf_(char *s, unsigned long count, const char *format, ...);
int vsnprintf_(char *s, unsigned long count, const char *format, va_list arg);
int fctprintf(void (*out)(char c, void *extra_arg), void *extra_arg, const char *format, ...);
int vfctprintf(void (*out)(char c, void *extra_arg), void *extra_arg, const char *format, va_list arg);
void fprintf(FILE *restrict stream, const char *restrict format, ...);
int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg);

// file operations
int fgetc(FILE *stream);
char *fgets(char *restrict s, int n, FILE *restrict stream);
int fputc(int c, FILE *stream);
int fputs(const char *restrict s, FILE *restrict stream);
int remove(const char *filename);
int rename(const char *old, const char *new);
FILE *tmpfile(void);
char *tmpnam(char *s);
int fclose(FILE *stream);
int fflush(FILE *stream);
long int ftell(FILE *stream);
FILE *fopen(const char *restrict filename, const char *restrict mode);
FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream);
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
int fseek(FILE *stream, long int offset, int whence);

// input
int sscanf(const char *restrict s, const char *restrict format, ...);

void puts(const char *str);
void putchar(int c);