#include <serial.h>

void SerialWritec(char c)
{
    outb(COM1,c); // output the character on the serial console
}

void SerialWrite(const char *str)
{
    while(*str) // loop thru every character
    {
        if(*str == '\n') SerialWritec('\r'); // the serial console uses the CRLF end line method and we don't 
        SerialWritec(*str);
        str++;
    }
}