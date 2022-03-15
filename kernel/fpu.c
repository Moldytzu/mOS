#include <fpu.h>

extern void SSEInit();
extern void x87Init();

void fpuInit()
{
    x87Init(); // init x87 in assembly
    SSEInit(); // init sse in assembly
}