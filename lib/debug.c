#include "debug.h"

void
error (const char *fmt, ...)
{
    char message[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(message);
    exit(1);

}

void
debug_log (const char *fmt, ...)
{
    char message[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(message, sizeof(message), fmt, ap);
    va_end(ap);
    time_t now = time(NULL);
    char *str_time = asctime(localtime(&now));
    str_time[strlen(str_time) - 1] = '\0';
    FILE *out = fopen(DEBUG_LOG_FILE, "a+");
    fprintf(out, "[%s] %s\n", str_time, message);
    fclose(out);
}