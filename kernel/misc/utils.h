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
#define align(val, alg) (max(val,alg) + (alg - (max(val,alg) % alg)))
#define alignD(val, alg) (align(val, alg) - alg)
#define unsafe_cast(val, type) (*(type *)&val)
#define iasm asm volatile
#define ifunc static inline __attribute__((always_inline))
#define doptimize __attribute__((optimize("O0")))
#define between(a, b, c) (((uint64_t)(a) >= (uint64_t)(b)) && ((uint64_t)(a) <= (uint64_t)(c)))
#define optimize __attribute__((target("sse4.2"), optimize("O3")))

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

// compare memory
ifunc int memcmp(const void *a, const void *b, size_t len)
{
    return memcmp8((void *)a, (void *)b, len);
}

// string length
ifunc uint32_t strlen(const char *str)
{
    uint32_t i = 0;
    for (; *str; str++, i++)
        ;
    return i;
}

// set memory 8 bits at a time
ifunc optimize void memset8(void *dest, uint8_t data, size_t count)
{
    for (; count; count--, dest++)
        *(uint8_t *)dest = data;
}

// set memory 16 bits at a time
ifunc optimize void memset16(void *dest, uint16_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint16_t))
        *(uint16_t *)dest = data;
}

// set memory 32 bits at a time
ifunc optimize void memset32(void *dest, uint32_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint32_t))
        *(uint32_t *)dest = data;
}

// set memory 64 bits at a time
ifunc optimize void memset64(void *dest, uint64_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint64_t))
        *(uint64_t *)dest = data;
}

// reverse a string
ifunc optimize void strrev(char *str)
{
    size_t len = strlen(str);
    for (int i = 0, j = len - 1; i < j; i++, j--)
    {
        const char a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

// copy memory (8 bits)
ifunc optimize void memcpy8(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
}

// copy memory (16 bits)
ifunc optimize void memcpy16(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint16_t *)dest)[i] = ((uint16_t *)src)[i];
}

// copy memory (32 bits)
ifunc optimize void memcpy32(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint32_t *)dest)[i] = ((uint32_t *)src)[i];
}

// copy memory (64 bits)
ifunc optimize void memcpy64(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint64_t *)dest)[i] = ((uint64_t *)src)[i];
}

// check if a string starts with something
ifunc optimize bool strstarts(const char *str, const char *start)
{
    if (strlen(str) < strlen(start)) // the lenght is smaller than the start
        return false;

    return memcmp(str, start, strlen(start)) == 0; // compare the start
}

// check if a string ends with something
ifunc optimize bool strendss(const char *str, const char *ends)
{
    if (strlen(str) < strlen(ends)) // the lenght is smaller than the start
        return false;

    return memcmp(str + strlen(str) - strlen(ends), ends, strlen(ends)) == 0; // compare the end
}

// compare string
ifunc optimize int strcmp(const char *str1, const char *str2)
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

// copy memory
ifunc void *memcpy(void *dest, const void *src, size_t num)
{
    memcpy8(dest, (void *)src, num);
    return dest;
}

// set memory
ifunc void *memset(void *dstpp, int c, size_t len)
{
    memset8(dstpp, c, len);
    return dstpp;
}

// move memory
ifunc optimize void *memmove(void *dest, const void *src, size_t n)
{
    uint8_t *from = (uint8_t *)src;
    uint8_t *to = (uint8_t *)dest;

    if (from == to || n == 0)
        return dest;
    if (to > from && to - from < n)
    {
        for (size_t i = n - 1; i >= 0; i--)
            to[i] = from[i];
        return dest;
    }
    if (from > to && from - to < n)
    {
        for (size_t i = 0; i < n; i++)
            to[i] = from[i];
        return dest;
    }
    memcpy(dest, src, n);
    return dest;
}

// to_string
const char *to_string(uint64_t val);
const char *to_hstring(uint64_t val);

// hang
void hang();

// printk
void printk(const char *fmt, ...);
void printks(const char *fmt, ...);

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