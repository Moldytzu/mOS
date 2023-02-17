#include <misc/logger.h>
#include <sched/time.h>
#include <cpu/smp.h>
#include <drv/framebuffer.h>

uint32_t logOldColour;

#define PUSH_COLOUR(x)                                             \
    {                                                              \
        framebuffer_cursor_info_t cursor = framebufferGetCursor(); \
        logOldColour = cursor.colour;                              \
        cursor.colour = x;                                         \
        framebufferSetCursor(cursor);                              \
    }

#define POP_COLOUR()                                               \
    {                                                              \
        framebuffer_cursor_info_t cursor = framebufferGetCursor(); \
        cursor.colour = logOldColour;                              \
        framebufferSetCursor(cursor);                              \
    }

void logInfo(const char *fmt, ...)
{
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

    va_list args;

    PUSH_COLOUR(0x3050FF);
    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds
    POP_COLOUR();

    PUSH_COLOUR(0x20BF20);
    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);
    POP_COLOUR();

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

    PUSH_COLOUR(0x3050FF);
    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds
    POP_COLOUR();

    PUSH_COLOUR(0xFFA000);
    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);
    POP_COLOUR();

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

    PUSH_COLOUR(0x3050FF);
    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds
    POP_COLOUR();

    PUSH_COLOUR(0xFF2020);
    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);
    POP_COLOUR();

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

    PUSH_COLOUR(0x00FF00);

    printk("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printk_impl(fmt, args);
    va_end(args);

    printk("\n");

    POP_COLOUR();

#ifdef K_COM_LOG
    printks("{#%d} (%d.%d): ", smpID(), milis / 1000, milis % 1000 / 100); // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

    va_start(args, fmt);
    printks_impl(fmt, args);
    va_end(args);

    printks("\n");
#endif
}