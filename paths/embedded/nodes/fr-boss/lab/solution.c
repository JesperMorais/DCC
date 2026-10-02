#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* Conveyor motor controller: 1 kHz control loop, UART commands, telemetry, software watchdog. */

#define CONTROL_PERIOD pdMS_TO_TICKS(1)
#define TELEMETRY_PERIOD pdMS_TO_TICKS(10)
#define WDOG_PERIOD pdMS_TO_TICKS(5)
#define WDOG_MIN_CHECKINS 3 /* the control loop checks in ~5 times per window */
#define MAX_RPM 1000
#define CMD_QUEUE_LEN 8
#define CTRL_LOG_LEN 1000

#define PRIO_CONTROL (configMAX_PRIORITIES - 1) /* 7: above the timer daemon (6) */
#define PRIO_CMD (tskIDLE_PRIORITY + 5)
#define PRIO_TELEMETRY (tskIDLE_PRIORITY + 2)

typedef enum { CMD_SET_SPEED, CMD_STOP } cmd_type_t;
typedef struct {
    cmd_type_t type;
    int32_t value; /* rpm, for CMD_SET_SPEED */
} motor_cmd_t;

QueueHandle_t g_cmd_q;
SemaphoreHandle_t g_setpoint_mutex;
TimerHandle_t g_wdog_timer;
TaskHandle_t g_control_task;

int32_t g_setpoint; /* rpm, protected by g_setpoint_mutex */
int32_t g_pwm;      /* written only by the control task */

uint32_t g_ctrl_cycles;
uint32_t g_ctrl_overruns;
TickType_t g_ctrl_tick[CTRL_LOG_LEN]; /* tick of each control cycle */

uint32_t g_cmds_applied;
uint32_t g_cmds_dropped;
TickType_t g_cmd_applied_tick;

uint32_t g_telemetry_frames;
int32_t g_telemetry_setpoint; /* setpoint in the last telemetry frame */

uint32_t g_checkins;
bool g_wd_tripped;
TickType_t g_wd_trip_tick;

/* ---- provided board support: don't change ---- */
TickType_t g_stall_at = portMAX_DELAY; /* tests inject an encoder hang here */
TickType_t g_stall_ticks = 20;
bool g_motor_enabled = true;

static void encoder_read(void) {
    if (xTaskGetTickCount() == g_stall_at) {
        sim_mark("encoder SPI hang");
        vTaskDelay(g_stall_ticks); /* waiting for a DMA-complete that comes late */
    }
}

static void motor_disable(void) { g_motor_enabled = false; /* gate driver off */ }

static void send_telemetry_frame(int32_t setpoint, int32_t pwm) {
    (void)pwm;
    vSimulateWork(3); /* format + CAN transmit: slow */
    g_telemetry_setpoint = setpoint;
    g_telemetry_frames++;
}
/* ---- end of provided code ---- */

static int32_t clamp_rpm(int32_t rpm) { return rpm < 0 ? 0 : rpm > MAX_RPM ? MAX_RPM : rpm; }

/* UART frame-complete interrupt: the driver hands us one decoded command. */
void uart_cmd_isr(const motor_cmd_t *cmd) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xQueueSendFromISR(g_cmd_q, cmd, &xHigherPriorityTaskWoken) != pdPASS) g_cmds_dropped++;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void control_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        if (g_ctrl_cycles < CTRL_LOG_LEN) g_ctrl_tick[g_ctrl_cycles] = xTaskGetTickCount();
        g_ctrl_cycles++;
        g_checkins++; /* we're the highest priority: nobody can interrupt this increment */

        encoder_read();
        xSemaphoreTake(g_setpoint_mutex, portMAX_DELAY);
        int32_t setpoint = g_setpoint;
        xSemaphoreGive(g_setpoint_mutex);
        g_pwm = (g_wd_tripped || !g_motor_enabled) ? 0 : setpoint;

        if (xTaskDelayUntil(&last_wake, CONTROL_PERIOD) == pdFALSE) {
            g_ctrl_overruns++;
            last_wake = xTaskGetTickCount(); /* resync: don't burst to catch up */
        }
    }
}

static void cmd_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        motor_cmd_t cmd;
        xQueueReceive(g_cmd_q, &cmd, portMAX_DELAY);
        int32_t rpm = cmd.type == CMD_SET_SPEED ? clamp_rpm(cmd.value) : 0;
        xSemaphoreTake(g_setpoint_mutex, portMAX_DELAY);
        g_setpoint = rpm;
        xSemaphoreGive(g_setpoint_mutex);
        g_cmd_applied_tick = xTaskGetTickCount();
        g_cmds_applied++;
    }
}

static void telemetry_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(g_setpoint_mutex, portMAX_DELAY);
        int32_t setpoint = g_setpoint; /* copy under the lock... */
        xSemaphoreGive(g_setpoint_mutex);
        send_telemetry_frame(setpoint, g_pwm); /* ...do the slow part outside it */
        xTaskDelayUntil(&last_wake, TELEMETRY_PERIOD);
    }
}

/* Timer daemon context: never blocks. */
static void wdog_cb(TimerHandle_t timer) {
    (void)timer;
    taskENTER_CRITICAL(); /* read-and-clear must not be split by a control cycle */
    uint32_t checkins = g_checkins;
    g_checkins = 0;
    taskEXIT_CRITICAL();
    if (checkins < WDOG_MIN_CHECKINS && !g_wd_tripped) {
        g_wd_tripped = true;
        g_wd_trip_tick = xTaskGetTickCount();
        motor_disable();
    }
}

void app_start(void) {
    g_cmd_q = xQueueCreate(CMD_QUEUE_LEN, sizeof(motor_cmd_t));
    g_setpoint_mutex = xSemaphoreCreateMutex();
    g_wdog_timer = xTimerCreate("wdog", WDOG_PERIOD, pdTRUE, NULL, wdog_cb);
    configASSERT(g_cmd_q && g_setpoint_mutex && g_wdog_timer);

    xTaskCreate(control_task, "control", configMINIMAL_STACK_SIZE * 4, NULL, PRIO_CONTROL, &g_control_task);
    xTaskCreate(cmd_task, "cmd", configMINIMAL_STACK_SIZE * 2, NULL, PRIO_CMD, NULL);
    xTaskCreate(telemetry_task, "telemetry", configMINIMAL_STACK_SIZE * 4, NULL, PRIO_TELEMETRY, NULL);
    xTimerStart(g_wdog_timer, 0);
}
