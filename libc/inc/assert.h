#pragma once

// page 181 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

#ifdef NDEBUG
#define assert(ignore)
#else
#define assert(expression) ((expression) ? NULL : __assert(#expression, __FILE__))
void __assert(const char *str, const char *file);
#endif