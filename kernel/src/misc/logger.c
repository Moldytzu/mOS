#include <misc/logger.h>
#include <sched/time.h>
#include <cpu/smp.h>

void logInfo(const char *fmt, ...)
{
    va_list args;
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");
}

void logWarn(const char *fmt, ...)
{
    va_list args;
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");
}

void logError(const char *fmt, ...)
{
    va_list args;
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");
}

void logDbg(int level, const char *fmt, ...)
{
    // todo: use level as a level of verbosity by comparing with the value set in config.h and returning if it is higher than that

    va_list args;
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");
}