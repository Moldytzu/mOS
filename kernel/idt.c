#include <idt.h>

void IDTInit()
{
    asm volatile ("cli"); // disable intrerrupts
    
    asm volatile ("sti"); // enable intrerrupts
}