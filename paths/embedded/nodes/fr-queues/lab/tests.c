#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "sim.h"
#include "task.h"

/* ---- the "UART": only the gatekeeper may call it ---- */
static char out[4096];
static int lines;
static TickType_t uart_ticks_per_line = 2;

void uart_write_line(const char *line) {
    const char *who = pcTaskGetName(NULL);
    if (strcmp(who, "uart_gk") != 0) sim_fail("uart_write_line() called from \"%s\": only the uart_gk gatekeeper may touch the UART", who);
    vSimulateWork(uart_ticks_per_line); /* bytes on the wire */
    strncat(out, line, sizeof out - strlen(out) - 2);
    strcat(out, "\n");
    lines++;
}

/* ---- test-side producer tasks ---- */
typedef struct {
    uint8_t source;
    char tag;
    int count;
    int failed;
    TickType_t finished_at;
} producer_t;

static void producer_task(void *pvParameters) {
    producer_t *p = pvParameters;
    for (int i = 0; i < p->count; i++) {
        char text[16];
        snprintf(text, sizeof text, "%c%d", p->tag, i);
        if (log_send(p->source, text) != pdPASS) p->failed++;
    }
    p->finished_at = xTaskGetTickCount();
    vTaskDelete(NULL);
}

static int pos(const char *needle) {
    const char *at = strstr(out, needle);
    return at ? (int)(at - out) : -1;
}

TEST(one_line_reaches_the_uart_through_the_gatekeeper) {
    static producer_t ui = {SRC_UI, 'u', 1, 0, 0};
    logger_init();
    xTaskCreate(producer_task, "ui", configMINIMAL_STACK_SIZE, &ui, 1, NULL);
    sim_run(20);
    EXPECT_STR_EQ(out, "U: u0\n");
}

TEST(three_tasks_lose_nothing_and_lines_stay_whole_and_in_order) {
    static producer_t motor = {SRC_MOTOR, 'm', 4, 0, 0}, sensor = {SRC_SENSOR, 's', 4, 0, 0}, ui = {SRC_UI, 'u', 4, 0, 0};
    logger_init();
    xTaskCreate(producer_task, "motor", configMINIMAL_STACK_SIZE, &motor, 5, NULL);
    xTaskCreate(producer_task, "sensor", configMINIMAL_STACK_SIZE, &sensor, 4, NULL);
    xTaskCreate(producer_task, "ui", configMINIMAL_STACK_SIZE, &ui, 1, NULL);
    sim_run(100);
    EXPECT_EQ(lines, 12);
    EXPECT_EQ(g_log_dropped, 0);
    EXPECT_TRUE(pos("M: m0\n") >= 0 && pos("M: m0\n") < pos("M: m3\n"));
    EXPECT_TRUE(pos("S: s0\n") >= 0 && pos("S: s0\n") < pos("S: s3\n"));
    EXPECT_TRUE(pos("U: u0\n") >= 0 && pos("U: u0\n") < pos("U: u3\n"));
}

TEST(a_motor_burst_of_8_lines_never_blocks_the_motor) {
    static producer_t motor = {SRC_MOTOR, 'm', 8, 0, 99};
    logger_init();
    xTaskCreate(producer_task, "motor", configMINIMAL_STACK_SIZE, &motor, 5, NULL);
    sim_run(50);
    EXPECT_EQ(motor.finished_at, 0); /* all 8 sends returned at once: the queue is big enough */
    EXPECT_EQ(lines, 8);
}

TEST(a_burst_just_over_the_queue_size_waits_briefly_instead_of_dropping) {
    static producer_t motor = {SRC_MOTOR, 'm', 10, 0, 99};
    logger_init();
    xTaskCreate(producer_task, "motor", configMINIMAL_STACK_SIZE, &motor, 5, NULL);
    sim_run(50);
    EXPECT_EQ(g_log_dropped, 0); /* the gatekeeper frees a slot within the 5 ms timeout */
    EXPECT_EQ(lines, 10);
    EXPECT_TRUE(motor.finished_at <= 5);
}

static void reuse_buffer_task(void *pvParameters) {
    (void)pvParameters;
    char buf[16] = "first";
    log_send(SRC_SENSOR, buf);
    strcpy(buf, "second"); /* overwrite immediately, like a real snprintf-and-send loop */
    log_send(SRC_SENSOR, buf);
    strcpy(buf, "garbage");
    vTaskDelete(NULL);
}

TEST(messages_are_copied_so_callers_can_reuse_their_buffer) {
    logger_init();
    xTaskCreate(reuse_buffer_task, "sensor", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    sim_run(20);
    EXPECT_STR_EQ(out, "S: first\nS: second\n");
}

TEST(a_stuck_uart_costs_the_motor_a_timeout_not_a_stall) {
    static producer_t motor = {SRC_MOTOR, 'm', 12, 0, 0};
    uart_ticks_per_line = 200; /* the UART is wedged: 200 ms per line */
    logger_init();
    xTaskCreate(producer_task, "motor", configMINIMAL_STACK_SIZE, &motor, 5, NULL);
    sim_run(2500);
    EXPECT_TRUE(motor.finished_at <= 20); /* blocked at most ~5 ms per dropped line */
    EXPECT_TRUE(g_log_dropped >= 1);
    EXPECT_EQ(motor.failed, (int)g_log_dropped); /* log_send reported every drop */
    EXPECT_EQ(lines + (int)g_log_dropped, 12);    /* everything else got through */
}
