#pragma once
#include <stddef.h>
#include <stdint.h>

int memcmp(const void *str1, const void *str2, size_t n);
void *memcpy(void *dest, const char *src, size_t n);
void *memset(void *str, int c, size_t n);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);