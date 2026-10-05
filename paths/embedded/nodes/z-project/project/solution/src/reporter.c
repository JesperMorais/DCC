/*
 * report_timer (ISR context) --k_work_submit--> report_work (system workqueue)
 *
 * The expiry runs inside the timer interrupt, so it only submits the work.
 * Taking the stats mutex and logging happen in the work handler, in a thread.
 */
#include <errno.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "reporter.h"
#include "stats.h"

LOG_MODULE_REGISTER(reporter, LOG_LEVEL_INF);

static void report_handler(struct k_work *work)
{
	struct stats s;
	char min[16], max[16], mean[16];

	ARG_UNUSED(work);

	stats_count_report();
	stats_get(&s);
	if (s.samples == 0) {
		LOG_INF("report: no samples yet");
		return;
	}

	format_milli(min, sizeof(min), s.min_mc);
	format_milli(max, sizeof(max), s.max_mc);
	format_milli(mean, sizeof(mean), s.sum_mc / s.samples);
	LOG_INF("report: %u samples, min %s, max %s, mean %s C, %u dropped", (unsigned int)s.samples, min, max, mean,
		(unsigned int)s.dropped);
}

K_WORK_DEFINE(report_work, report_handler);

static void report_expiry(struct k_timer *timer)
{
	ARG_UNUSED(timer);
	k_work_submit(&report_work);
}

K_TIMER_DEFINE(report_timer, report_expiry, NULL);

int reporter_set_period(uint32_t period_ms)
{
	if (period_ms < REPORTER_MIN_MS || period_ms > REPORTER_MAX_MS) {
		return -EINVAL;
	}
	/* Periodic from the start: restarting a one-shot from the handler would drift. */
	k_timer_start(&report_timer, K_MSEC(period_ms), K_MSEC(period_ms));
	return 0;
}

static int reporter_init(void)
{
	return reporter_set_period(CONFIG_SENSOR_NODE_REPORT_MS);
}

SYS_INIT(reporter_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
