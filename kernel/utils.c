#include <utils.h>
#include <framebuffer.h>

uint32_t strlen(const char *str)
{
    uint32_t i = 0;
    for (; *str; str++, i++)
        ;
    return i;
}

void hang()
{
    iasm("cli"); // disable intrerrupts
    for (;;)
        iasm("hlt"); // halt
}

void memset(void *dest, uint8_t data, size_t count)
{
    for (; count; count--, dest++)
        *(uint8_t *)dest = data;
}

void memset16(void *dest, uint16_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint16_t))
        *(uint16_t *)dest = data;
}

void memset32(void *dest, uint32_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint32_t))
        *(uint32_t *)dest = data;
}

void memset64(void *dest, uint64_t data, size_t count)
{
    for (; count; count--, dest += sizeof(uint64_t))
        *(uint64_t *)dest = data;
}

int memcmp(void *a, void *b, size_t len)
{
    for (size_t i = 0; i < len; i++, a++, b++)
    {
        if (*(uint8_t *)a != *(uint8_t *)b)
            return *(uint8_t *)a - *(uint8_t *)b;
    }
    return 0;
}

void strrev(char *str)
{
    size_t len = strlen(str);
    for (int i = 0, j = len - 1; i < j; i++, j--)
    {
        const char a = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

char to_stringout[32];
const char *to_string(uint64_t val)
{
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    memset64(to_stringout, 0, 4); // clear output, (64/8)*4 = 32 bytes
    for (int i = 0; val; i++, val /= 10)
        to_stringout[i] = (val % 10) + '0';

    strrev(to_stringout); // reverse string

    return to_stringout;
}

char to_hstringout[32];
const char *to_hstring(uint64_t val)
{
    const char *digits = "0123456789ABCDEF";
    if (!val)
        return "0"; // if the value is 0 then return a constant string "0"

    memset64(to_stringout, 0, 4); // clear output, (64/8)*4 = 32 bytes

    uint8_t nibbles = 0;
    if (val > UINT32_MAX)
        nibbles = 16; // it's 64 bit
    else if (val > UINT16_MAX)
        nibbles = 8; // it's 32 bit
    else if (val > UINT8_MAX)
        nibbles = 4; // it's 16 bit
    else
        nibbles = 2;

    for (int i = 0; i < nibbles; i++, val = val >> 4) // shift the value by 4 to get each nibble
        to_hstringout[i] = digits[val & 0xF];         // get each nibble

    strrev(to_hstringout); // reverse string

    return to_hstringout;
}

void printk(const char *fmt, ...)
{
    va_list list;
    va_start(list, fmt); // start a variable arguments list

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            if (*fmt == 'd')
                framebufferWrite(to_string(va_arg(list, uint64_t))); // decimal
            else if (*fmt == 'p')
                framebufferWrite(to_hstring((uint64_t)va_arg(list, void *))); // pointer
            else if (*fmt == 'x')
                framebufferWrite(to_hstring(va_arg(list, uint64_t))); // hex
            else if (*fmt == 's')
                framebufferWrite(va_arg(list, const char *)); // string
        }
        else
            framebufferWritec(*fmt);
        fmt++;
    }

    va_end(list); // clean up
}

void memcpy(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
}

void memcpy16(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint16_t *)dest)[i] = ((uint16_t *)src)[i];
}

void memcpy32(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint32_t *)dest)[i] = ((uint32_t *)src)[i];
}

void memcpy64(void *dest, void *src, size_t count)
{
    for (size_t i = 0; i < count; i++)
        ((uint64_t *)dest)[i] = ((uint64_t *)src)[i];
}

void cli()
{
    iasm("cli");
}

void sti()
{
    iasm("sti");
}