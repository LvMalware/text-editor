#ifndef _COMMON_H
#define _COMMON_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef DEBUG_LOG_FILE
#define DEBUG_LOG_FILE "debug.log"
#endif

void error (const char *fmt, ...);

void debug_log (const char *fmt, ...);

#endif /* common.h */