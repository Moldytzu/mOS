#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <config.h>
#include <cpu/atomic.h>
#include <misc/data.h>

// useful macros
#define bitsof(x) (sizeof(x) * 8)
#define align(val, alg) (max((uint64_t)(val), alg) + (alg - (max((uint64_t)(val), alg) % alg))) // deprecated!
#define alignD(val, alg) (align(val, alg) - alg)                                                // deprecated!
#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define between(a, b, c) (((uint64_t)(a) >= (uint64_t)(b)) && ((uint64_t)(a) <= (uint64_t)(c)))
#define pstruct typedef struct __attribute__((__packed__))
#define align_addr(al) __attribute__((aligned(al)))

#ifdef K_SMP
#define lock(l, cmds)      \
    {                      \
        atomicAquire(&l);  \
        cmds;              \
        atomicRelease(&l); \
    }
#define release(l)         \
    {                      \
        atomicRelease(&l); \
    }
#else
#define lock(l, cmds) {cmds}
#define release(l)
#endif

// compare memory
ifunc int memcmp8(void *a, void *b, size_t len)
{
    for (size_t i = 0; i < len; i++, a++, b++)
    {
        if (*(uint8_t *)a != *(uint8_t *)b)
            return *(uint8_t *)a - *(uint8_t *)b;
    }
    return 0;
}

int memcmp(const void *a, const void *b, size_t len);

// string length
ifunc uint32_t strlen(const char *str)
{
    uint32_t i = 0;
    for (; *str; str++, i++)
        ;
    return i;
}

// set memory 8 bits at a time
static void memset8(void *dest, uint8_t data, size_t count)
{
    for (; count; count--, dest++)
        *(uint8_t *)dest = data;
}

// set memory 16 bits at a time
static void memset16(void *dest, uint16_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint16_t))
        *(uint16_t *)dest = data;
}

// set memory 32 bits at a time
static void memset32(void *dest, uint32_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint32_t))
        *(uint32_t *)dest = data;
}

// set memory 64 bits at a time
static void memset64(void *dest, uint64_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint64_t))
        *(uint64_t *)dest = data;
}

// set memory to 0
static void zero(void *dest, size_t count)
{
    if (count % sizeof(uint64_t) == 0)
        memset64(dest, 0, count / sizeof(uint64_t));
    else if (count % sizeof(uint32_t) == 0)
        memset32(dest, 0, count / sizeof(uint32_t));
    else if (count % sizeof(uint16_t) == 0)
        memset16(dest, 0, count / sizeof(uint16_t));
    else
        memset8(dest, 0, count);
}

// reverse a string
ifunc void strrev(char *str)
{
    size_t len = strlen(str);
    for (int i = 0, j = len - 1; i < j; i++, j--)
    {
        const char a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

// lower an uppercase character
char tolower(char c)
{
    if(between(c, 'A', 'Z'))
        return c + 32;
    
    return c;
}

// copy memory (8 bits)
ifunc void memcpy8(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
}

// copy memory (16 bits)
ifunc void memcpy16(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint16_t *)dest)[i] = ((uint16_t *)src)[i];
}

// copy memory (32 bits)
ifunc void memcpy32(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint32_t *)dest)[i] = ((uint32_t *)src)[i];
}

// copy memory (64 bits)
ifunc void memcpy64(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint64_t *)dest)[i] = ((uint64_t *)src)[i];
}

// check if a string starts with something
ifunc bool strstarts(const char *str, const char *start)
{
    if (strlen(str) < strlen(start)) // the lenght is smaller than the start
        return false;

    return memcmp(str, start, strlen(start)) == 0; // compare the start
}

// check if a string ends with something
ifunc bool strendss(const char *str, const char *ends)
{
    if (strlen(str) < strlen(ends)) // the lenght is smaller than the start
        return false;

    return memcmp(str + strlen(str) - strlen(ends), ends, strlen(ends)) == 0; // compare the end
}

// compare string
ifunc int strcmp(const char *str1, const char *str2)
{
    while (*str1)
    {
        if (*str2 != *str1)
            break;

        str1++;
        str2++;
    }

    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

void *memcpy(void *dest, const void *src, size_t num);
void *memset(void *dstpp, int c, size_t len);
void *memmove(void *dest, const void *src, size_t n);

// to_string
const char *to_string(uint64_t val);
const char *to_hstring(uint64_t val);

// hang
void hang();

// printk
void printk(const char *fmt, ...);
void printks(const char *fmt, ...);
void printk_impl(const char *fmt, va_list list);
void printks_impl(const char *fmt, va_list list);

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

ifunc int abs(int a)
{
    if (a < 0)
        return -a;
    return a;
}