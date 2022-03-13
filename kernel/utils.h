#include <stddef.h>
#include <stdint.h>

#define pack __attribute__((__packed__))

uint32_t strlen(const char *str);
void hang();