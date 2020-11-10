#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

#define LOGGER_BUFFER_SIZE  (200)

typedef struct
{
    uint8_t length;
    uint8_t data[LOGGER_BUFFER_SIZE];
} buffer_t;

extern UART_HandleTypeDef huart1;
static buffer_t buffer;

void logger(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    buffer.length = vsnprintf((char *) buffer.data, sizeof(buffer.data), format, args);
    va_end(args);

    HAL_UART_Transmit(&huart1, buffer.data, buffer.length, HAL_MAX_DELAY);
}