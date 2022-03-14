#include <utils.h>

uint32_t strlen(const char *str)
{
    uint32_t i = 0;
    for (; *str; str++, i++)
        ;
    return i;
}

void hang()
{
    asm volatile("cli"); // disable intrerrupts
    for (;;)
        asm volatile("hlt"); // halt
}

void memset(void *dest, uint8_t data, size_t count)
{
    for (size_t i = 0; i < count; i++, dest++)
        *(uint8_t *)dest = data;
}