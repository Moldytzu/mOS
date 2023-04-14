#include <misc/logger.h>
#include <sched/time.h>
#include <cpu/smp.h>
#include <drv/framebuffer.h>

locker_t loggerLock;
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

#define DO_WRITE()                                                                                             \
    {                                                                                                          \
        printk("[c%d] (%d.%d%d%d) ", smpID(), milis / 1000, milis % 1000 / 100, milis % 100 / 10, milis % 10); \
    } // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

#define DO_WRITES()                                                                                             \
    {                                                                                                           \
        printks("[c%d] (%d.%d%d%d) ", smpID(), milis / 1000, milis % 1000 / 100, milis % 100 / 10, milis % 10); \
    } // do some trickery so we don't use the fpu, display first the seconds part then hundreds of miliseconds

void logInfo(const char *fmt, ...)
{
    lock(loggerLock, {
        uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

        va_list args;

        // print our prefix
        PUSH_COLOUR(0x3050FF);
        DO_WRITE();
        POP_COLOUR();

        // print format given
        PUSH_COLOUR(0x20BF20);
        va_start(args, fmt);
        printk_impl(fmt, args);
        va_end(args);
        POP_COLOUR();

        // don't forget a new line
        printk("\n");

#ifdef K_COM_LOG
        // do same thing but on serial
        DO_WRITES();

        va_start(args, fmt);
        printks_impl(fmt, args);
        va_end(args);

        printks("\n");
#endif
    });
}

void logWarn(const char *fmt, ...)
{
    lock(loggerLock, {
        va_list args;
        uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

        // print our prefix
        PUSH_COLOUR(0x3050FF);
        DO_WRITE();
        POP_COLOUR();

        // print given format
        PUSH_COLOUR(0xFFA000);
        va_start(args, fmt);
        printk_impl(fmt, args);
        va_end(args);
        POP_COLOUR();

        printk("\n");

#ifdef K_COM_LOG
        // do same thing but on serial
        DO_WRITES();

        va_start(args, fmt);
        printks_impl(fmt, args);
        va_end(args);

        printks("\n");
#endif
    });
}

void logError(const char *fmt, ...)
{
    lock(loggerLock, {
        va_list args;
        uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

        // print our prefix
        PUSH_COLOUR(0x3050FF);
        DO_WRITE();
        POP_COLOUR();

        // print given format
        PUSH_COLOUR(0xFF2020);
        va_start(args, fmt);
        printk_impl(fmt, args);
        va_end(args);
        POP_COLOUR();

        printk("\n");

#ifdef K_COM_LOG
        // do same thing on serial
        DO_WRITES();

        va_start(args, fmt);
        printks_impl(fmt, args);
        va_end(args);

        printks("\n");
#endif
    });
}

void logDbg(int level, const char *fmt, ...)
{
    lock(loggerLock, {
        va_list args;
        uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

        if (level != LOG_SERIAL_ONLY)
        {
            // print our prefix and the given format
            PUSH_COLOUR(0x00FF00);
            DO_WRITE();

            va_start(args, fmt);
            printk_impl(fmt, args);
            va_end(args);

            printk("\n");

            POP_COLOUR();
        }

#ifdef K_COM_LOG
        // print on serial our prefix and given format
        DO_WRITES();

        va_start(args, fmt);
        printks_impl(fmt, args);
        va_end(args);

        printks("\n");
#endif
    });
}