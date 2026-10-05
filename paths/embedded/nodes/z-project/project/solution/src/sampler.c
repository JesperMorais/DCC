/*
 * sampler (every SENSOR_NODE_SAMPLE_MS) --k_msgq--> processing --> stats
 *
 * The sampler never waits for the queue: if processing falls behind, the
 * sample is dropped and counted, and the sampling period stays steady.
 */
#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sampler.h"
#include "stats.h"

LOG_MODULE_REGISTER(sampler, LOG_LEVEL_INF);

struct sample {
	int32_t milli_c;
	uint32_t taken_ms;
};

K_MSGQ_DEFINE(sample_q, sizeof(struct sample), CONFIG_SENSOR_NODE_QUEUE_LEN, 4);

/* The sampler and the shell both read the sensor; fetch + get must be one step. */
K_MUTEX_DEFINE(sensor_lock);

static const struct device *const ambient = DEVICE_DT_GET(DT_ALIAS(ambient_temp));

int sampler_read(int32_t *milli_c)
{
	struct sensor_value val;
	int err;

	if (!device_is_ready(ambient)) {
		return -ENODEV;
	}

	k_mutex_lock(&sensor_lock, K_FOREVER);
	err = sensor_sample_fetch(ambient);
	if (err == 0) {
		err = sensor_channel_get(ambient, SENSOR_CHAN_AMBIENT_TEMP, &val);
	}
	k_mutex_unlock(&sensor_lock);

	if (err == 0) {
		*milli_c = val.val1 * 1000 + val.val2 / 1000;
	}
	return err;
}

static void sampler_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(ambient)) {
		LOG_ERR("%s is not ready, not sampling", ambient->name);
		return;
	}

	/* Absolute deadlines: a late wake-up doesn't push every later sample back. */
	int64_t next = k_uptime_get();

	for (;;) {
		struct sample s = {.taken_ms = k_uptime_get_32()};
		int err = sampler_read(&s.milli_c);

		if (err != 0) {
			LOG_WRN("reading %s failed: %d", ambient->name, err);
		} else if (k_msgq_put(&sample_q, &s, K_NO_WAIT) != 0) {
			stats_count_drop();
		}

		next += CONFIG_SENSOR_NODE_SAMPLE_MS;
		k_sleep(K_TIMEOUT_ABS_MS(next));
	}
}

static void processing_thread(void *p1, void *p2, void *p3)
{
	struct sample s;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	for (;;) {
		k_msgq_get(&sample_q, &s, K_FOREVER);
		stats_add(s.milli_c);
		LOG_DBG("sample %d mC, %u ms in the queue", s.milli_c, k_uptime_get_32() - s.taken_ms);
	}
}

/* Both preemptible. Processing is more urgent (lower number), so the queue drains first. */
K_THREAD_DEFINE(sampler_tid, 1024, sampler_thread, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(processing_tid, 1024, processing_thread, NULL, NULL, NULL, 5, 0, 0);
