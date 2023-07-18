#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_PRIME 100000
#define PAGES 32768 // 128 megabytes

volatile int count = 0;

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

int main(int argc, char **argv)
{
    uint64_t a, b, c, unused;

    count = 0;
    a = sys_time_uptime_nanos();

    // check for primes
    for (volatile int i = 2; i < MAX_PRIME; i++)
    {
        if (isPrime(i))
            count++;
    }

    b = sys_time_uptime_nanos();

    for (volatile int i = 0; i < PAGES; i++)
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&unused, 0);

    c = sys_time_uptime_nanos();

    printf("found %d primes up to %d in %llu miliseconds\n", count, MAX_PRIME, (b - a) / 1000000);
    printf("allocated %d pages in %llu miliseconds\n", PAGES, (c - b) / 1000000);
}