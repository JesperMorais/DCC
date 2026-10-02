#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "sim.h"
#include "task.h"
#include "timers.h"

/* ---- the "UART": each USART1 frame interrupt delivers the next scripted command ---- */
static motor_cmd_t script[8];
static int next_cmd;
static void usart1_frame(void) { uart_cmd_isr(&script[next_cmd++]); }

static void command_at(rtos_tick_t tick, cmd_type_t type, int32_t value) {
    static int n;
    script[n++] = (motor_cmd_t){type, value};
    sim_irq_at(tick, "USART1", usart1_frame);
}

/* ---- a CAN/comms stack at priority 4 that eats 60% of the CPU ---- */
static void comms_task(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        vSimulateWork(3);
        vTaskDelay(2);
    }
}

static void start_loaded(void) {
    app_start();
    xTaskCreate(comms_task, "comms", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
}

TEST(control_runs_every_millisecond_under_load) {
    start_loaded();
    sim_run(500);
    EXPECT_EQ(g_ctrl_cycles, 500);
    for (uint32_t n = 0; n < 500; n++) EXPECT_EQ(g_ctrl_tick[n], n);
    EXPECT_EQ(g_ctrl_overruns, 0);
    EXPECT_TRUE(g_telemetry_frames >= 45); /* telemetry still gets its slots */
    EXPECT_FALSE(g_wd_tripped);           /* a healthy loop never trips the watchdog */
}

TEST(control_outranks_everything_including_the_timer_daemon) {
    app_start();
    EXPECT_NOT_NULL(g_control_task);
    EXPECT_EQ(uxTaskPriorityGet(g_control_task), configMAX_PRIORITIES - 1);
    EXPECT_TRUE(uxTaskPriorityGet(g_control_task) > configTIMER_TASK_PRIORITY);
}

TEST(a_command_is_applied_in_the_tick_its_interrupt_fires) {
    start_loaded();
    command_at(100, CMD_SET_SPEED, 500);
    sim_run(103);
    EXPECT_EQ(g_cmds_applied, 1);
    EXPECT_EQ(g_cmd_applied_tick, 100); /* 101 means no portYIELD_FROM_ISR */
    EXPECT_EQ(g_setpoint, 500);
    EXPECT_EQ(g_pwm, 500); /* the control loop picked it up on its next cycle */
}

TEST(commands_apply_in_order_and_are_clamped) {
    start_loaded();
    command_at(200, CMD_SET_SPEED, 300);
    command_at(210, CMD_SET_SPEED, 5000);
    command_at(220, CMD_SET_SPEED, -40);
    command_at(230, CMD_SET_SPEED, 700);
    command_at(240, CMD_STOP, 0);
    sim_run(205);
    EXPECT_EQ(g_pwm, 300);
    sim_run(10);
    EXPECT_EQ(g_pwm, MAX_RPM);
    sim_run(10);
    EXPECT_EQ(g_pwm, 0);
    sim_run(10);
    EXPECT_EQ(g_pwm, 700);
    sim_run(10);
    EXPECT_EQ(g_pwm, 0);
    EXPECT_EQ(g_cmds_applied, 5);
}

TEST(no_priority_inversion_on_the_setpoint_lock) {
    start_loaded();
    command_at(95, CMD_SET_SPEED, 400);
    command_at(181, CMD_SET_SPEED, 600);
    command_at(262, CMD_SET_SPEED, 650);
    sim_run(400);
    /* If telemetry holds the lock across its 3-tick frame, control has to wait,
     * and telemetry shows up here with control's inherited priority. */
    EXPECT_EQ(sim_max_priority_seen("telemetry"), 2);
    EXPECT_EQ(sim_max_priority_seen("cmd"), 5);
    EXPECT_EQ(g_ctrl_cycles, 400);
    EXPECT_EQ(g_telemetry_setpoint, 650);
}

TEST(watchdog_catches_an_encoder_stall_and_makes_the_motor_safe) {
    start_loaded();
    command_at(50, CMD_SET_SPEED, 800);
    g_stall_at = 300; /* control blocks 300..320 inside encoder_read() */
    sim_run(290);
    EXPECT_EQ(g_pwm, 800);
    EXPECT_FALSE(g_wd_tripped);
    sim_run(160);
    EXPECT_TRUE(g_wd_tripped);
    EXPECT_TRUE(g_wd_trip_tick > 300 && g_wd_trip_tick <= 311); /* within ~2 windows */
    EXPECT_FALSE(g_motor_enabled);
    EXPECT_EQ(g_pwm, 0); /* and the control loop latches the fault */
}

TEST(after_a_stall_the_loop_resyncs_instead_of_bursting) {
    start_loaded();
    g_stall_at = 300;
    sim_run(400);
    EXPECT_EQ(g_ctrl_overruns, 1);              /* counted from xTaskDelayUntil's return */
    EXPECT_EQ(g_ctrl_tick[300], 300);           /* the stalled cycle */
    EXPECT_EQ(g_ctrl_tick[301], 320);           /* resumes at 320... */
    EXPECT_EQ(g_ctrl_tick[302], 321);           /* ...and is back on 1 ms, no catch-up burst */
    for (uint32_t n = 1; n < g_ctrl_cycles && n < CTRL_LOG_LEN; n++) EXPECT_TRUE(g_ctrl_tick[n] > g_ctrl_tick[n - 1]);
}
