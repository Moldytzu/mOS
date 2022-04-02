#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <config.h>

#define pack __attribute__((__packed__))
#define toMB(x) (x / 1024 / 1024)
#define toKB(x) (x / 1024)
#define align(val, alg) (val + (alg - val % alg))
#define alignD(val, alg) (val - (alg - val % alg))
#define unsafe_cast(val, type) (*(type *)&val)
#define iasm asm volatile

// strlen
uint32_t strlen(const char *str);

// strrev
void strrev(char *str);

// memsets
void memset(void *dest, uint8_t data, size_t count);
void memset16(void *dest, uint16_t data, size_t count);
void memset32(void *dest, uint32_t data, size_t count);
void memset64(void *dest, uint64_t data, size_t count);

// memcpys
void memcpy(void *dest, void *src, size_t count);
void memcpy16(void *dest, void *src, size_t count);
void memcpy32(void *dest, void *src, size_t count);
void memcpy64(void *dest, void *src, size_t count);

// memcmp
int memcmp(void *a, void *b, size_t len);

// to_string
const char *to_string(uint64_t val);
const char *to_hstring(uint64_t val);

// hang
void hang();

// printk
void printk(const char *fmt, ...);

// intrerrupts control
void cli();
void sti();