#include <fpu.h>

extern void sseInit();
extern void x87Init();

void fpuInit()
{
    x87Init(); // init x87 in assembly
    sseInit(); // init sse in assembly
}