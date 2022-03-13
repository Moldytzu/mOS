#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define pack __attribute__((__packed__))

uint32_t strlen(const char *str);
void hang();