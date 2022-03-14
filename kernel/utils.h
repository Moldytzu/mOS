#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define pack __attribute__((__packed__))

// strlen
uint32_t strlen(const char *str);

// memsets
void memset(void *dest, uint8_t data, size_t count);
void memset16(void *dest, uint16_t data, size_t count);
void memset32(void *dest, uint32_t data, size_t count);
void memset64(void *dest, uint64_t data, size_t count);

// memcmp
int memcmp(void *a, void *b, size_t len);

// hang
void hang();