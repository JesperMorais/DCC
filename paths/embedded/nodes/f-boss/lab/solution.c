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

/* Deadline first: the alarm must be on the dot, the parser must drain 4-byte bursts
 * before the next frame, and the uplink is background work below the display. */
#define PRIO_ALARM 4
#define PRIO_PARSER 3
#define PRIO_UPLINK 1
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

/* ---- the data lock: a mutex, so the kernel knows the owner and can lend it priority ---- */
static rtos_mutex_t *g_data_lock;
static void data_lock(void) { rtos_mutex_lock(g_data_lock, RTOS_WAIT_FOREVER); }
static void data_unlock(void) { rtos_mutex_unlock(g_data_lock); }

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
    USART1->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE;
    GPIOB->MODER = (GPIOB->MODER & ~(3u << (ALARM_PIN * 2u))) | (1u << (ALARM_PIN * 2u)); /* 01: output */
    GPIOB->ODR &= ~(1u << ALARM_PIN);
}

/* One interrupt per received byte. */
void USART1_IRQHandler(void) {
    uint32_t sr = USART1->SR;
    if (!(sr & USART_SR_RXNE)) return;
    uint8_t byte = (uint8_t)USART1->DR;
    USART1->SR &= ~(USART_SR_RXNE | USART_SR_ORE); /* acknowledge, even if we drop it */
    if (!rtos_queue_send(g_byte_q, &byte, RTOS_NO_WAIT)) g_bytes_dropped++;
}

static void store_reading(int16_t t) {
    data_lock();
    g_stats.latest = t;
    g_stats.latest_tick = rtos_now();
    g_stats.frames++;
    if (t < g_stats.min) g_stats.min = t;
    if (t > g_stats.max) g_stats.max = t;
    data_unlock();
}

static void parser_task(void *arg) {
    (void)arg;
    parse_state_t state = WAIT_SYNC;
    uint8_t msb = 0, lsb = 0;
    for (;;) {
        uint8_t b;
        rtos_queue_receive(g_byte_q, &b, RTOS_WAIT_FOREVER);
        switch (state) {
        case WAIT_SYNC:
            if (b == SYNC_BYTE) state = GOT_SYNC;
            break;
        case GOT_SYNC:
            msb = b;
            state = GOT_MSB;
            break;
        case GOT_MSB:
            lsb = b;
            state = GOT_LSB;
            break;
        case GOT_LSB:
            state = WAIT_SYNC;
            if (b != (msb ^ lsb)) {
                g_bad_frames++;
                break;
            }
            store_reading(calibrate((int16_t)(uint16_t)((msb << 8) | lsb))); /* slow part outside the lock */
            break;
        }
    }
}

static void alarm_task(void *arg) {
    (void)arg;
    rtos_tick_t next = rtos_now();
    for (;;) {
        alarm_self_check();
        data_lock();
        int16_t latest = g_stats.latest;
        rtos_tick_t latest_tick = g_stats.latest_tick;
        uint32_t frames = g_stats.frames;
        data_unlock();

        bool lost = rtos_now() - latest_tick > SENSOR_TIMEOUT;
        bool out_of_range = frames > 0 && (latest < TEMP_LOW || latest > TEMP_HIGH);
        if (lost || out_of_range) GPIOB->ODR |= 1u << ALARM_PIN;
        else GPIOB->ODR &= ~(1u << ALARM_PIN);

        rtos_delay_until(&next, ALARM_PERIOD); /* drift-free: the self-check doesn't push the grid */
    }
}

static void uplink_task(void *arg) {
    (void)arg;
    rtos_tick_t next = rtos_now();
    for (;;) {
        rtos_delay_until(&next, UPLINK_PERIOD);
        data_lock();
        report_t r = {0, g_stats.frames, g_stats.min, g_stats.max};
        g_stats.min = INT16_MAX; /* start the next window in the same hold: no frame falls between */
        g_stats.max = INT16_MIN;
        data_unlock();
        modem_send(&r); /* 40 ms, never under the lock */
    }
}

void monitor_start(void) {
    hw_init();
    g_byte_q = rtos_queue_create(BYTE_Q_LEN, sizeof(uint8_t));
    g_data_lock = rtos_mutex_create(true);
    rtos_task_create("alarm", alarm_task, NULL, PRIO_ALARM);
    rtos_task_create("parser", parser_task, NULL, PRIO_PARSER);
    rtos_task_create("display", display_task, NULL, PRIO_DISPLAY);
    rtos_task_create("eeprom", eeprom_task, NULL, PRIO_EEPROM);
    rtos_task_create("uplink", uplink_task, NULL, PRIO_UPLINK);
}
