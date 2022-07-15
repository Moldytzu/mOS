#include <cpu/fpu.h>
#include <cpuid.h>

extern void sseInit();
extern void x87Init();

// init the floating point unit and the simds
void fpuInit()
{
    x87Init(); // init x87 in assembly
    sseInit(); // init sse in assembly
}

// checks for newer sse versions to be available and supported
bool fpuCheck()
{
    uint32_t versionInfo, additionalInfo, featureInfo, featureInfo2;
    __get_cpuid(1, &versionInfo, &additionalInfo, &featureInfo2, &featureInfo); // get information
    return (featureInfo2 >> 20) & 1; // test for bit 20 in ECX (sse4.2)
}