/* Vaccine-fridge monitor: a temperature sensor streams frames over USART1, the firmware
 * decodes them, sounds an alarm on PB5 when the fridge leaves 2.0..8.0 °C or the sensor
 * goes silent, and sends a report over the cellular modem every second. */
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcu.h"
#include "rtos.h"
#include "sim.h"

#define SYNC_BYTE 0xA5u     /* frame: SYNC, MSB, LSB, CHK (= MSB ^ LSB) */
#define BYTE_Q_LEN 8        /* bytes buffered between the ISR and the parser */
#define ALARM_PERIOD 100    /* ms */
#define UPLINK_PERIOD 1000  /* ms */
#define SENSOR_TIMEOUT 300  /* ms without a valid frame = sensor lost */
#define TEMP_LOW 20         /* 2.0 °C, in tenths of a degree */
#define TEMP_HIGH 80        /* 8.0 °C */
#define ALARM_PIN 5u        /* PB5: buzzer + red LED, active high */
#define MAX_REPORTS 8

/* TODO: pick these. The display (2) and the EEPROM writer (1) are fixed. */
#define PRIO_ALARM 1
#define PRIO_PARSER 1
#define PRIO_UPLINK 3
#define PRIO_DISPLAY 2
#define PRIO_EEPROM 1

typedef struct {
    int16_t latest;          /* tenths of a degree */
    rtos_tick_t latest_tick; /* when `latest` was stored */
    uint32_t frames;         /* valid frames since boot */
    int16_t min, max;        /* since the last uplink report */
} fridge_stats_t;

typedef struct {
    rtos_tick_t tick; /* filled in by modem_send() */
    uint32_t frames;
    int16_t min, max;
} report_t;

typedef enum { WAIT_SYNC, GOT_SYNC, GOT_MSB, GOT_LSB } parse_state_t;

rtos_queue_t *g_byte_q;                                    /* uint8_t: ISR -> parser */
fridge_stats_t g_stats = {.min = INT16_MAX, .max = INT16_MIN}; /* shared: only touch it under the data lock */
uint32_t g_bytes_dropped;                                  /* written only by the ISR */
uint32_t g_bad_frames;                                     /* written only by the parser */

/* ---- the data lock: written in a hurry, review it ---- */
static rtos_sem_t *g_lock_sem;
static void data_lock(void) { rtos_sem_take(g_lock_sem, RTOS_WAIT_FOREVER); }
static void data_unlock(void) { rtos_sem_give(g_lock_sem); }

/* ---- provided: don't change ---- */
report_t g_reports[MAX_REPORTS];
unsigned g_nreports;
uint32_t g_eeprom_frames;

static int16_t calibrate(int16_t raw) {
    rtos_busy(1); /* linearisation table lookup; this unit's offset is zero */
    return raw;
}

static void alarm_self_check(void) {
    rtos_busy(2); /* buzzer driver + sensor plausibility check */
}

static void modem_send(report_t *r) {
    sim_mark("uplink: modem transmitting");
    r->tick = rtos_now();
    if (g_nreports < MAX_REPORTS) g_reports[g_nreports] = *r;
    g_nreports++;
    rtos_busy(40); /* AT commands + LTE-M transmit: slow */
}

/* E-ink redraw: 70 ms of CPU every 200 ms, from tick 35. */
static void display_task(void *arg) {
    (void)arg;
    rtos_tick_t next = 0;
    rtos_delay_until(&next, 35);
    for (;;) {
        sim_mark("display: e-ink redraw");
        rtos_busy(70);
        rtos_delay_until(&next, 200);
    }
}

/* Legacy: persists the frame counter. The EEPROM driver needs the lock held across the 3 ms page write. */
static void eeprom_task(void *arg) {
    (void)arg;
    rtos_tick_t next = 0;
    rtos_delay_until(&next, 234);
    for (;;) {
        data_lock();
        sim_mark("eeprom: page write, holding the data lock");
        g_eeprom_frames = g_stats.frames;
        rtos_busy(3);
        data_unlock();
        rtos_delay_until(&next, 500);
    }
}
/* ---- end of provided code ---- */

static void hw_init(void) {
    /* TODO: enable USART1 (UE), its receiver (RE) and its RX interrupt (RXNEIE),
     * make PB5 an output (MODER 01) and drive it low. Leave every other bit alone. */
}

/* One interrupt per received byte. */
void USART1_IRQHandler(void) {
    /* TODO: if RXNE is set, read DR, acknowledge (clear RXNE and ORE in SR) and
     * queue the byte for the parser. Never wait; count a full queue in g_bytes_dropped. */
}

static void parser_task(void *arg) {
    (void)arg;
    for (;;) {
        /* TODO: receive bytes, step a parse_state_t state machine. A frame is
         * SYNC_BYTE, MSB, LSB, CHK. If CHK != MSB ^ LSB, count g_bad_frames and go back to
         * waiting for SYNC. Otherwise the reading is the int16_t (MSB << 8) | LSB:
         * calibrate() it, then store it in g_stats under the data lock
         * (latest, latest_tick = now, frames++, min, max). */
        (void)calibrate;
        rtos_delay(RTOS_WAIT_FOREVER);
    }
}

static void alarm_task(void *arg) {
    (void)arg;
    for (;;) {
        /* TODO, every ALARM_PERIOD on the dot:
         *  - alarm_self_check()
         *  - copy what you need from g_stats under the data lock
         *  - alarm if the last valid frame is older than SENSOR_TIMEOUT (from boot if none
         *    yet), or if there is a reading and it's outside TEMP_LOW..TEMP_HIGH
         *  - drive PB5 high (alarm) or low, without touching the other GPIOB pins */
        (void)alarm_self_check;
        rtos_delay(RTOS_WAIT_FOREVER);
    }
}

static void uplink_task(void *arg) {
    (void)arg;
    for (;;) {
        /* TODO, every UPLINK_PERIOD (first report at 1000): take a report_t of
         * frames, min and max, and reset min/max for the next window, all under the
         * data lock. Then modem_send() it. */
        (void)modem_send;
        rtos_delay(RTOS_WAIT_FOREVER);
    }
}

void monitor_start(void) {
    hw_init();
    g_byte_q = rtos_queue_create(BYTE_Q_LEN, sizeof(uint8_t));
    g_lock_sem = rtos_sem_create(1, 1);
    rtos_task_create("alarm", alarm_task, NULL, PRIO_ALARM);
    rtos_task_create("parser", parser_task, NULL, PRIO_PARSER);
    rtos_task_create("display", display_task, NULL, PRIO_DISPLAY);
    rtos_task_create("eeprom", eeprom_task, NULL, PRIO_EEPROM);
    rtos_task_create("uplink", uplink_task, NULL, PRIO_UPLINK);
}
