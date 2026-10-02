#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Drive controller logging: many tasks log, but only the gatekeeper owns the UART. */

#define LOG_QUEUE_LEN 8                   /* the motor logs bursts of up to 8 lines */
#define LOG_SEND_TIMEOUT pdMS_TO_TICKS(5) /* never stall a caller longer than this */
#define LOG_TEXT_LEN 24

enum { SRC_MOTOR, SRC_SENSOR, SRC_UI };

typedef struct {
    uint8_t source;
    char text[LOG_TEXT_LEN];
} log_msg_t;

QueueHandle_t g_log_q;
uint32_t g_log_dropped;

/* Board support: writes one line to the UART. Slow, and NOT thread-safe. */
void uart_write_line(const char *line);

static char source_letter(uint8_t source) {
    switch (source) {
        case SRC_MOTOR: return 'M';
        case SRC_SENSOR: return 'S';
        default: return 'U';
    }
}

static void uart_gatekeeper_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        log_msg_t msg;
        if (xQueueReceive(g_log_q, &msg, portMAX_DELAY) != pdPASS) continue;
        char line[LOG_TEXT_LEN + 4];
        snprintf(line, sizeof line, "%c: %s", source_letter(msg.source), msg.text);
        uart_write_line(line); /* the only caller of the UART, ever */
    }
}

BaseType_t log_send(uint8_t source, const char *text) {
    log_msg_t msg = {.source = source};
    snprintf(msg.text, sizeof msg.text, "%s", text); /* copy: the caller may reuse its buffer */
    if (xQueueSend(g_log_q, &msg, LOG_SEND_TIMEOUT) != pdPASS) {
        g_log_dropped++;
        return pdFAIL;
    }
    return pdPASS;
}

void logger_init(void) {
    g_log_q = xQueueCreate(LOG_QUEUE_LEN, sizeof(log_msg_t));
    configASSERT(g_log_q != NULL);
    xTaskCreate(uart_gatekeeper_task, "uart_gk", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 2, NULL);
}
