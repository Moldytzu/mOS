#include <libbenchmark.h>
#include <mos/sys.h>

// todo: a context system would be great
uint64_t benchmarkStartNanos;

void benchmarkStart()
{
    benchmarkStartNanos = sys_time_uptime_nanos();
}

uint64_t benchmarkEnd()
{
    uint64_t difference = sys_time_uptime_nanos() - benchmarkStartNanos;
    uint64_t miliseconds = difference / 1000000; // difference is in nanoseconds
    return miliseconds;
}
