#ifndef LOG_H__
#define LOG_H__

#include <stdint.h>

#define LOG_LEVEL_DEFAULT   LOG_LEVEL_DEBUG

#define LOG_LEVEL_NO        (0)
#define LOG_LEVEL_ERROR     (1)
#define LOG_LEVEL_WARN      (2)
#define LOG_LEVEL_INFO      (3)
#define LOG_LEVEL_DEBUG     (4)

#define LOG_COLOR_ERROR     "\033[0;31m"
#define LOG_COLOR_WARN      "\033[0;33m"
#define LOG_COLOR_INFO      "\033[0;34m"
#define LOG_COLOR_DEBUG     "\033[0;32m"

#if (LOG_LEVEL_DEFAULT == LOG_LEVEL_DEBUG)
    #define DBG(_format_, ...) log_printf(LOG_COLOR_DEBUG, __LINE__, _format_, ##__VA_ARGS__)
#else
    #define DBG(...)
#endif

#if (LOG_LEVEL_DEFAULT >= LOG_LEVEL_INFO)
    #define LOG(_format_, ...) log_printf(LOG_COLOR_INFO, __LINE__, _format_, ##__VA_ARGS__)
#else
    #define LOG(...)
#endif

#if (LOG_LEVEL_DEFAULT >= LOG_LEVEL_WARN)
    #define WRN(_format_, ...) log_printf(LOG_COLOR_WARN, __LINE__, _format_, ##__VA_ARGS__)
#else
    #define WRN(...)
#endif

#if (LOG_LEVEL_DEFAULT >= LOG_LEVEL_ERROR)
    #define ERR(_format_, ...) log_printf(LOG_COLOR_ERROR, __LINE__, _format_, ##__VA_ARGS__)
#else
    #define ERR(...)
#endif

void log_init(void);
void log_printf(const char *color, int line, const char *format, ...);
#endif /* LOG_H__ */