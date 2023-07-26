#include <mos/sys.h>
#include <libbenchmark.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// benchmark settings
#define PRIMES 100000 // number of primes to find
#define PAGES 32768   // 128 megabytes

// fast prime checker
bool isPrime(uint64_t n)
{
    if (n != 2 && n % 2 == 0) // even numbers larger than 2 are not prime!
        return false;

    uint64_t i = 2;

    while (i * i <= n)
    {
        if (n % i == 0) // n has a factor between 2 and sqrt(n)
            return false;
        i++;
    }

    return true;
}

void primes()
{
    volatile size_t count = 0;
    benchmark_t b;

    BENCHMARK(&b, {
        // check for primes
        for (volatile uint64_t i = 2; i < UINT64_MAX; i++)
        {
            if (isPrime(i))
                count++;

            if (count == PRIMES)
                break;
        }
    });

    printf("found %d primes in %llu miliseconds\n", count, b.elapsedMiliseconds);
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