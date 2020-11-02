#ifndef LOG_H__
#define LOG_H__

#define LOG(__format, ...)  logger(__format, ##__VA_ARGS__)

void logger(const char * format, ...);

#endif /* LOG_H__ */