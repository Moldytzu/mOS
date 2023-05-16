#pragma once
#include <misc/utils.h>

// logDbg
#define LOG_ALWAYS 0      // always logs on both screen and serial
#define LOG_SERIAL_ONLY 1 // always logs on serial
#define LOG_DBG_VERBOSE 2 // logs only if global verbosity level is higher than VERBOSE
#define LOG_DBG_MAX 3     // logs only if global verbosity level is set to MAX

void logInfo(const char *fmt, ...);
void logWarn(const char *fmt, ...);
void logError(const char *fmt, ...);
void logDbg(int level, const char *fmt, ...);