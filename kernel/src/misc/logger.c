#include <misc/logger.h>
#include <sched/time.h>
#include <cpu/smp.h>

// todo: add colours!

void logInfo(const char *fmt, ...)
{
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    va_list args;

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");

#ifdef K_COM_LOG
    printks("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printks_impl(fmt, args);
    va_end(args);

    printks("\n");
#endif
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

#ifdef K_COM_LOG
    printks("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printks_impl(fmt, args);
    va_end(args);

    printks("\n");
#endif
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

#ifdef K_COM_LOG
    printks("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printks_impl(fmt, args);
    va_end(args);

    printks("\n");
#endif
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

#ifdef K_COM_LOG
    printks("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printks_impl(fmt, args);
    va_end(args);

    printks("\n");
#endif
}