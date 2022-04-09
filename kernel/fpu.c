#include <fpu.h>

extern void sseInit();
extern void x87Init();

// init the floating point unit and the simds
void fpuInit()
{
    x87Init(); // init x87 in assembly
    sseInit(); // init sse in assembly
}