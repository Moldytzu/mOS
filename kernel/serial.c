#include <serial.h>

// write string on the serial console
void serialWrite(const char *str)
{
    while (*str) // loop thru every character
    {
        if (*str == '\n')
            serialWritec('\r'); // the serial console uses the CRLF end line method and we don't
        serialWritec(*str);
        str++;
    }
}