#pragma once
#include <stddef.h>
#include <stdint.h>

// page 337 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

int memcmp(const void *str1, const void *str2, size_t n);
void *memcpy(void *dest, const char *src, size_t n);
void *memset(void *str, int c, size_t n);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
void strrev(char *str);