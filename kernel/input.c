#include <input.h>

char kbBuffer[4096]; // keyboard buffer
int kbIndex = 0;

void kbAppendChar(char c)
{
    kbBuffer[kbIndex++] = c; // append the character

    if (kbIndex == 4096) // prevent buffer overflow
        kbIndex = 0;
}

char kbGetLastKey()
{
    char last = kbBuffer[kbIndex];
    kbBuffer[kbIndex--] = '\0'; // clear the character

    if (kbIndex < 0) // prevent buffer underflow
        kbIndex = 0;

    return last;
}

char *kbGetBuffer()
{
    return kbBuffer;
}

void inputInit()
{
    memset64(kbBuffer, 0, sizeof(kbBuffer) / sizeof(uint64_t)); // clear the buffer
}