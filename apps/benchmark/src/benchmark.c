#include <mos/sys.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

volatile int count = 0;

// very slow prime checker
bool isPrime(uint32_t n)
{
    if(n != 2 && n % 2 == 0) // even numbers larger than 2 are not prime!
        return false;

    for(int i = 2; i < n; i ++)
    {
        if(n % i == 0)
            return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    uint64_t old, new;
    sys_time(SYS_TIME_GET_UPTIME_NANOS, (uint64_t)&old, 0);

    for(volatile int i = 1; i < 50000; i++)
    {
        if(isPrime(i))
            count++;
    }

    sys_time(SYS_TIME_GET_UPTIME_NANOS, (uint64_t)&new, 0);

    printf("found %d primes in %llu miliseconds\n", count, (new - old) / 1000000);
}