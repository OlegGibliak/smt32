#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "main.h"

#include "FreeRTOS.h"
#include "log.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#define LOG_MUTEX_TIMEOUT_MS  (10)
#define LOG_QUEUE_TIMEOUT_MS  (10)

#define LOG_BUFF_SIZE         (128)
#define LOG_QUEUE_LENGTH      (10)
#define LOG_ITEM_SIZE         (sizeof(log_msg_t))

#define LOG_COLOR_SIZE        (sizeof(LOG_COLOR_ERROR) - 1)

#define SUBSEC_TO_MS(_subsec_, _fract_) (((_fract_ - _subsec_) * 1000) / (_fract_ + 1))

typedef struct
{
    uint32_t len;
    char     buf[LOG_BUFF_SIZE];
} log_msg_t;

extern UART_HandleTypeDef huart1;

static QueueHandle_t      h_log_queue;
static uint8_t            log_queue_storage[LOG_QUEUE_LENGTH * LOG_ITEM_SIZE];
static StaticQueue_t      log_queue_ctb;

static SemaphoreHandle_t  h_lock;
static StaticSemaphore_t  lock_buf;

static TaskHandle_t       h_log_task;
static StackType_t        log_stack[configMINIMAL_STACK_SIZE];
static StaticTask_t       log_tcb;

static log_msg_t          m_msg;

void log_task(void *argument)
{
    log_msg_t msg;

    for (;;)
    {
        if (xQueueReceive(h_log_queue, &msg, portMAX_DELAY))
        {
            if (HAL_UART_Transmit(&huart1, (uint8_t *)msg.buf, msg.len, HAL_MAX_DELAY) == HAL_OK)
            {
                // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            }
        }
    }
}

// void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
// {
//     vTaskNotifyGiveFromISR(h_log_task, NULL);
// }

void log_init(void)
{
    h_log_queue = xQueueCreateStatic(LOG_QUEUE_LENGTH, LOG_ITEM_SIZE, log_queue_storage, &log_queue_ctb);
    assert_param(h_log_queue);

    h_lock = xSemaphoreCreateMutexStatic(&lock_buf);
    assert_param(h_lock);

    h_log_task = xTaskCreateStatic(log_task, "log_task", configMINIMAL_STACK_SIZE, NULL, 3, log_stack, &log_tcb);
    assert_param(h_log_task);
}

void log_printf(const char *color, int line, const char *format, ...)
{
    if (xSemaphoreTake(h_lock, LOG_MUTEX_TIMEOUT_MS) != pdPASS)
        return;

    m_msg.len = LOG_COLOR_SIZE;
    memcpy(m_msg.buf, color, LOG_COLOR_SIZE);
#if 0
    if (HAL_RTC_GetTime(&hrtc, &m_time, RTC_FORMAT_BIN) == HAL_OK &&
        HAL_RTC_GetDate(&hrtc, &m_date, RTC_FORMAT_BIN) == HAL_OK)
    {
        m_msg.len += sprintf(m_msg.buf + m_msg.len,
                             "[%02u-%02u-%02u][%02u:%02u:%02u.%03lu]",
                             m_date.Date,  m_date.Month,   m_date.Year,
                             m_time.Hours, m_time.Minutes, m_time.Seconds,
                             SUBSEC_TO_MS(m_time.SubSeconds, m_time.SecondFraction));
        
    }
#endif

    va_list args;
    va_start(args, format);

    m_msg.len += vsnprintf(&m_msg.buf[m_msg.len], LOG_BUFF_SIZE - m_msg.len, format, args);
    
    va_end(args);

    xQueueSendToBack(h_log_queue, &m_msg, LOG_QUEUE_TIMEOUT_MS);

    xSemaphoreGive(h_lock);
}