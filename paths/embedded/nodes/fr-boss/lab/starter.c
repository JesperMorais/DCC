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

#define PRIO_CONTROL (tskIDLE_PRIORITY + 1) /* TODO: the most urgent task in the system */
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
    /* TODO: queue *cmd for cmd_task (count a full queue in g_cmds_dropped) so
     * that cmd_task applies it in THIS tick. */
    (void)cmd;
}

static void control_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        /* TODO, every 1 ms, drift-free:
         *  - log the tick into g_ctrl_tick[g_ctrl_cycles] (while it fits), g_ctrl_cycles++
         *  - check in with the watchdog (g_checkins++)
         *  - encoder_read()
         *  - read g_setpoint under g_setpoint_mutex
         *  - g_pwm = setpoint, or 0 once the watchdog has tripped
         *  - wait for the next period; count an overrun in g_ctrl_overruns and
         *    resync instead of bursting to catch up */
        (void)encoder_read;
        vTaskDelay(CONTROL_PERIOD);
    }
}

static void cmd_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        /* TODO: receive a motor_cmd_t, clamp SET_SPEED to 0..MAX_RPM (STOP = 0),
         * write g_setpoint under the mutex, record g_cmd_applied_tick, g_cmds_applied++ */
        vTaskDelay(portMAX_DELAY);
    }
}

/* Written by a colleague. Code review flagged it. Check it before you ship. */
static void telemetry_task(void *pvParameters) {
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(g_setpoint_mutex, portMAX_DELAY);
        send_telemetry_frame(g_setpoint, g_pwm);
        xSemaphoreGive(g_setpoint_mutex);
        xTaskDelayUntil(&last_wake, TELEMETRY_PERIOD);
    }
}

/* Timer daemon context: never blocks. */
static void wdog_cb(TimerHandle_t timer) {
    (void)timer;
    /* TODO: read-and-clear g_checkins atomically; if fewer than WDOG_MIN_CHECKINS
     * arrived, trip once: g_wd_tripped, g_wd_trip_tick, motor_disable(). */
    (void)motor_disable;
    (void)clamp_rpm;
}

void app_start(void) {
    g_cmd_q = xQueueCreate(CMD_QUEUE_LEN, sizeof(motor_cmd_t));
    g_setpoint_mutex = xSemaphoreCreateMutex();
    /* TODO: create the auto-reload watchdog timer (WDOG_PERIOD) and start it */
    configASSERT(g_cmd_q && g_setpoint_mutex);

    xTaskCreate(control_task, "control", configMINIMAL_STACK_SIZE * 4, NULL, PRIO_CONTROL, &g_control_task);
    xTaskCreate(cmd_task, "cmd", configMINIMAL_STACK_SIZE * 2, NULL, PRIO_CMD, NULL);
    xTaskCreate(telemetry_task, "telemetry", configMINIMAL_STACK_SIZE * 4, NULL, PRIO_TELEMETRY, NULL);
    (void)wdog_cb;
}
