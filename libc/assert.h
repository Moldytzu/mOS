#pragma once

#ifdef NDEBUG
#define assert(ignore)
#else
void assert(int expression);
#endif