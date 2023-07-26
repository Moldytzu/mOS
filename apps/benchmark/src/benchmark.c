#include <mos/sys.h>
#include <libbenchmark.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// actual benchmarking tests
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
    benchmark_t b;

    BENCHMARK(&b, {
        // check for primes
        for (volatile int i = 2; i < MAX_PRIME; i++)
        {
            if (isPrime(i))
                count++;
        }
    });

    printf("found %d primes up to %d in %llu miliseconds\n", count, MAX_PRIME, b.elapsedMiliseconds);
}

void allocation()
{
    uint64_t unused;
    benchmark_t b;

    BENCHMARK(&b, {
        for (volatile int i = 0; i < PAGES; i++)
            sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&unused, 0);
    });

    printf("allocated %d pages in %llu miliseconds\n", PAGES, b.elapsedMiliseconds);
}

int main(int argc, char **argv)
{
    primes();
    allocation();
}