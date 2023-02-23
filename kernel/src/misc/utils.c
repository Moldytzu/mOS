#include <misc/utils.h>
#include <drv/framebuffer.h>
#include <drv/serial.h>

locker_t utilsLock;

// stop everything
void hang()
{
    cli(); // disable intrerrupts
    for (;;)
        hlt(); // halt
}

// convert to a string (base 10)
char to_stringout[32];
const char *to_string(uint64_t val)
{
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    zero(to_stringout, sizeof(to_stringout)); // clear output
    for (int i = 0; val; i++, val /= 10)
        to_stringout[i] = (val % 10) + '0';

    strrev(to_stringout); // reverse string

    return to_stringout;
}

// convert to a string (base 16)
char to_hstringout[32];
const char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    zero(to_stringout, sizeof(to_hstringout)); // clear output

    for (int i = 0; i < 16; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];    // get each nibble

    strrev(to_hstringout); // reverse string

    // move the pointer until the first valid digit
    uint8_t offset = 0;
    for (; to_hstringout[offset] == '0'; offset++)
        ;

    return to_hstringout + offset; // return the string
}

// print formated on framebuffer
void printk_impl(const char *fmt, va_list list)
{
    lock(utilsLock, {
        for (size_t i = 0; fmt[i]; i++)
        {
            if (fmt[i] != '%')
            {
                framebufferWritec(fmt[i]);
                continue;
            }
            if (fmt[i + 1] == 'd')
                framebufferWrite(to_string(va_arg(list, uint64_t))); // decimal
            else if (fmt[i + 1] == 'p')
                framebufferWrite(to_hstring((uint64_t)va_arg(list, void *))); // pointer
            else if (fmt[i + 1] == 'x')
                framebufferWrite(to_hstring(va_arg(list, uint64_t))); // hex
            else if (fmt[i + 1] == 's')
                framebufferWrite(va_arg(list, const char *)); // string
            else if (fmt[i + 1] == 'c')
                framebufferWritec(va_arg(list, int)); // char
            i++;
        }
    });
}

// print formated on serial
void printks_impl(const char *fmt, va_list list)
{
    lock(utilsLock, {
        for (size_t i = 0; fmt[i]; i++)
        {
            if (fmt[i] != '%')
            {
                serialWritec(fmt[i]);
                continue;
            }

            if (fmt[i + 1] == 'd')
                serialWrite(to_string(va_arg(list, uint64_t))); // decimal
            else if (fmt[i + 1] == 'p')
                serialWrite(to_hstring((uint64_t)va_arg(list, void *))); // pointer
            else if (fmt[i + 1] == 'x')
                serialWrite(to_hstring(va_arg(list, uint64_t))); // hex
            else if (fmt[i + 1] == 's')
                serialWrite(va_arg(list, const char *)); // string
            else if (fmt[i + 1] == 'c')
                serialWritec(va_arg(list, int)); // char
            i++;
        }
    });
}

void printk(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    printk_impl(fmt, list);
    va_end(list);
}

void printks(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    printks_impl(fmt, list);
    va_end(list);
}

// copy memory
void *memcpy(void *dest, const void *src, size_t num)
{
    memcpy8(dest, (void *)src, num);
    return dest;
}

// set memory
void *memset(void *dstpp, int c, size_t len)
{
    memset8(dstpp, c, len);
    return dstpp;
}

// move memory
void *memmove(void *dest, const void *src, size_t n)
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

// compare memory
int memcmp(const void *a, const void *b, size_t len)
{
    return memcmp8((void *)a, (void *)b, len);
}