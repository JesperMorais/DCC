#include "FreeRTOS.h"
#include "mcu.h"
#include "queue.h"
#include "task.h"

/* Barcode scanner on USART1: every byte must reach the decoder in the same tick. */

#define RX_QUEUE_LEN 8
#define RX_LOG_LEN 64

QueueHandle_t g_rx_queue;
char g_rx_buf[RX_LOG_LEN + 1];        /* bytes the decoder handled, in order */
TickType_t g_rx_tick[RX_LOG_LEN];     /* tick at which the decoder got each byte */
uint32_t g_rx_count;
uint32_t g_rx_dropped;                /* bytes lost because the queue was full */

/* ---- provided: don't change ---- */
static uint8_t uart_read_dr(void) {
    uint8_t b = (uint8_t)USART1->DR;
    USART1->SR &= ~USART_SR_RXNE; /* on a real STM32, reading DR clears RXNE */
    return b;
}

static void ui_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) vSimulateWork(100); /* redraws the display, never sleeps */
}
/* ---- end of provided code ---- */

void USART1_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t byte = uart_read_dr();
    if (xQueueSendFromISR(g_rx_queue, &byte, &xHigherPriorityTaskWoken) != pdPASS) g_rx_dropped++;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void decoder_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        uint8_t byte;
        xQueueReceive(g_rx_queue, &byte, portMAX_DELAY);
        if (g_rx_count < RX_LOG_LEN) {
            g_rx_tick[g_rx_count] = xTaskGetTickCount();
            g_rx_buf[g_rx_count] = (char)byte;
        }
        g_rx_count++;
        vSimulateWork(1); /* decode */
    }
}

void app_main(void) {
    g_rx_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(uint8_t));
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE;
    xTaskCreate(decoder_task, "decoder", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(ui_task, "ui", configMINIMAL_STACK_SIZE * 2, NULL, tskIDLE_PRIORITY + 1, NULL);
}
