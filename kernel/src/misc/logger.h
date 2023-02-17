#pragma once
#include <misc/utils.h>

void logInfo(const char *fmt, ...);
void logWarn(const char *fmt, ...);
void logError(const char *fmt, ...);
void logDbg(int level, const char *fmt, ...);