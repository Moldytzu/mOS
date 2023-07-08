#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX 100000

volatile int count = 0;

// slow prime checker
bool isPrime(uint32_t n)
{
    if(n != 2 && n % 2 == 0) // even numbers larger than 2 are not prime!
        return false;

    for(int i = 3; i < n; i += 2)
    {
        if(n % i == 0)
            return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    uint64_t old, new;

    count = 0;
    old = sys_time_uptime_nanos();

    for(volatile int i = 2; i < MAX; i++)
    {
        if(isPrime(i))
            count++;
    }

    new = sys_time_uptime_nanos();

    printf("found %d primes up to %d in %llu miliseconds\n", count, MAX, (new - old) / 1000000);
}