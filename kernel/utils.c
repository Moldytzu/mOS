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

void memset16(void *dest, uint16_t data, size_t count)
{
    for (size_t i = 0; i < count; i++, dest+=sizeof(uint16_t))
        *(uint16_t *)dest = data;
}

void memset32(void *dest, uint32_t data, size_t count)
{
    for (size_t i = 0; i < count; i++, dest+=sizeof(uint32_t))
        *(uint32_t *)dest = data;
}

void memset64(void *dest, uint64_t data, size_t count)
{
    for (size_t i = 0; i < count; i++, dest+=sizeof(uint64_t))
        *(uint64_t *)dest = data;
}

int memcmp(void *a, void *b, size_t len)
{
    for(size_t i = 0; i < len; i++, a++, b++)
    {
        if(*(uint8_t*)a != *(uint8_t*)b)
            return *(uint8_t*)a-*(uint8_t*)b;
    }
    return 0;
}