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
    for (; count; count--, dest++)
        *(uint8_t *)dest = data;
}

void memset16(void *dest, uint16_t data, size_t count)
{
    for (; count; count--, dest+=sizeof(uint16_t))
        *(uint16_t *)dest = data;
}

void memset32(void *dest, uint32_t data, size_t count)
{
    for (; count; count--, dest+=sizeof(uint32_t))
        *(uint32_t *)dest = data;
}

void memset64(void *dest, uint64_t data, size_t count)
{
    for (; count; count--, dest+=sizeof(uint64_t))
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

char to_stringout[32]; 
const char *to_string(uint64_t val)
{
    uint64_t check = val; // variable we will process to count the digits
    uint8_t digits = 0; // we store the digits here
    for(; check; digits++, check /= 10); // cut last digit while counting it

    memset64(to_stringout,0,4); // clear output, (64/8)*4 = 32 bytes
    to_stringout[0] = '0'; // set a '0'
    for(int i = digits-1; val; i--, val /= 10)
        to_stringout[i] = (val % 10) + '0';

    return to_stringout;
}