#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <config.h>

// useful macros
#define pack __attribute__((__packed__))
#define toMB(x) ((x) / 1024 / 1024)
#define toKB(x) ((x) / 1024)
#define align(val, alg) (val + (alg - val % alg))
#define alignD(val, alg) (val - (alg - val % alg))
#define unsafe_cast(val, type) (*(type *)&val)
#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define doptimize __attribute__((optimize("O0")))

// strlen
uint32_t strlen(const char *str);

// strrev
void strrev(char *str);

// memsets
void *memset(void *dstpp, int c, size_t len);
void memset8(void *dest, uint8_t data, size_t count);
void memset16(void *dest, uint16_t data, size_t count);
void memset32(void *dest, uint32_t data, size_t count);
void memset64(void *dest, uint64_t data, size_t count);

// memcpys
void *memcpy(void *dest, const void *src, size_t num);
void memcpy8(void *dest, void *src, size_t count);
void memcpy16(void *dest, void *src, size_t count);
void memcpy32(void *dest, void *src, size_t count);
void memcpy64(void *dest, void *src, size_t count);

// memcmp
int memcmp(const void *a, const void *b, size_t len);
int memcmp8(void *a, void *b, size_t len);

// to_string
const char *to_string(uint64_t val);
const char *to_hstring(uint64_t val);

// hang
void hang();

// printk
void printk(const char *fmt, ...);
void printks(const char *fmt, ...);

// strstarts, strends
bool strstarts(const char *str, const char *start);
bool strendss(const char *str, const char *ends);

// inline assembly shortcuts
ifunc void cli()
{
    iasm("cli");
}

ifunc void sti()
{
    iasm("sti");
}

ifunc void hlt()
{
    iasm("hlt");
}

// basic maths
ifunc int min(int a, int b)
{
    if (a > b)
        return b;
    
    return a;
}

ifunc int max(int a, int b)
{
    if (a > b)
        return a;
    
    return b;
}