#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// todo: move this in a dedicated library
// todo: before doing that, create a context system
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

#define MAX_PRIME 100000
#define PAGES 32768 // 128 megabytes

// slow prime checker
bool isPrime(uint32_t n)
{
    if (n != 2 && n % 2 == 0) // even numbers larger than 2 are not prime!
        return false;

    for (int i = 3; i < n; i += 2)
    {
        if (n % i == 0)
            return false;
    }

    return true;
}

void primes()
{
    volatile size_t count = 0;

    benchmarkStart();
    // check for primes
    for (volatile int i = 2; i < MAX_PRIME; i++)
    {
        if (isPrime(i))
            count++;
    }
    uint64_t miliseconds = benchmarkEnd();

    printf("found %d primes up to %d in %llu miliseconds\n", count, MAX_PRIME, miliseconds);
}

void allocation()
{
    benchmarkStart();

    uint64_t unused;
    for (volatile int i = 0; i < PAGES; i++)
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&unused, 0);

    uint64_t miliseconds = benchmarkEnd();

    printf("allocated %d pages in %llu miliseconds\n", PAGES, miliseconds);
}

int main(int argc, char **argv)
{
    primes();
    allocation();
}