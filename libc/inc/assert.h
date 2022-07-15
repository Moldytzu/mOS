#pragma once

#ifdef NDEBUG
#define assert(ignore)
#else
#define assert(expression) ((expression) ? NULL : __assert(#expression, __FILE__))
void __assert(const char *str, const char *file);
#endif