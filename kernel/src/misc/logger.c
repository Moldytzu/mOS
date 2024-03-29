#include <misc/logger.h>
#include <sched/time.h>
#include <cpu/smp.h>
#include <drv/framebuffer.h>

spinlock_t loggerLock;
uint32_t logOldColour;

#define PREFIX_COLOUR 0xD0D0D0
#define DEBUG_COLOUR 0x3BADFF
#define INFO_COLOUR 0x10B010
#define WARN_COLOUR 0xD07000
#define ERROR_COLOUR 0xFF2020

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

#ifndef K_LOG_QUIET
#define PRINT_PREFIX_FORMAT(x, y)                                                                              \
    {                                                                                                          \
        PUSH_COLOUR(x);                                                                                        \
        printk("[c%d] (%d.%d%d%d) ", smpID(), milis / 1000, milis % 1000 / 100, milis % 100 / 10, milis % 10); \
        POP_COLOUR();                                                                                          \
                                                                                                               \
        PUSH_COLOUR(y);                                                                                        \
        va_start(args, fmt);                                                                                   \
        printk_impl(fmt, args);                                                                                \
        va_end(args);                                                                                          \
        POP_COLOUR();                                                                                          \
        printk("\n");                                                                                          \
    } // print prefix then a format with desired colour
#else
#define PRINT_PREFIX_FORMAT(x, y)
#endif

#define PRINT_SERIAL(c)                                                                                                                                                     \
    {                                                                                                                                                                       \
        printks("\033[38;2;%d;%d;%dm[c%d] (%d.%d%d%d) ", c >> 16 & 0xFF, c >> 8 & 0xFF, c & 0xFF, smpID(), milis / 1000, milis % 1000 / 100, milis % 100 / 10, milis % 10); \
        va_start(args, fmt);                                                                                                                                                \
        printks_impl(fmt, args);                                                                                                                                            \
        va_end(args);                                                                                                                                                       \
        printks("\n");                                                                                                                                                      \
    } // print prefix then format on serial

#define BOILERPLATE \
    va_list args;   \
    uint64_t milis = TIME_NANOS_TO_MILIS(timeNanos());

void logInfo(const char *fmt, ...)
{
    lock(loggerLock, {
        BOILERPLATE;
        PRINT_PREFIX_FORMAT(PREFIX_COLOUR, INFO_COLOUR); // print our prefix
#ifdef K_COM_LOG
        PRINT_SERIAL(INFO_COLOUR); // do the printing on serial
#endif
    });
#if defined(K_FB_DOUBLE_BUFFER) && !defined(K_LOG_QUIET)
    framebufferUpdate();
#endif
}

void logWarn(const char *fmt, ...)
{
    lock(loggerLock, {
        BOILERPLATE;
        PRINT_PREFIX_FORMAT(PREFIX_COLOUR, WARN_COLOUR); // print our prefix
#ifdef K_COM_LOG
        PRINT_SERIAL(WARN_COLOUR); // do the printing on serial
#endif
    });
#if defined(K_FB_DOUBLE_BUFFER) && !defined(K_LOG_QUIET)
    framebufferUpdate();
#endif
}

void logError(const char *fmt, ...)
{
    lock(loggerLock, {
        BOILERPLATE;
        PRINT_PREFIX_FORMAT(PREFIX_COLOUR, ERROR_COLOUR); // print our prefix
#ifdef K_COM_LOG
        PRINT_SERIAL(ERROR_COLOUR); // do the printing on serial
#endif
    });
#if defined(K_FB_DOUBLE_BUFFER) && !defined(K_LOG_QUIET)
    framebufferUpdate();
#endif
}

void logDbg(int level, const char *fmt, ...)
{
    lock(loggerLock, {
        BOILERPLATE;
        if (level != LOG_SERIAL_ONLY)
            PRINT_PREFIX_FORMAT(DEBUG_COLOUR, DEBUG_COLOUR); // print our prefix
#ifdef K_COM_LOG
        PRINT_SERIAL(DEBUG_COLOUR); // do the printing on serial
#endif
    });
#if defined(K_FB_DOUBLE_BUFFER) && !defined(K_LOG_QUIET)
    framebufferUpdate();
#endif
}