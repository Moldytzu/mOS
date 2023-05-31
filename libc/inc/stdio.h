#pragma once
#include <stdarg.h>

// page 274 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

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

void puts(const char *str);
void putchar(int c);