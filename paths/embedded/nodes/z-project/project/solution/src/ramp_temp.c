/*
 * Driver for "vnd,ramp-temp": a pretend temperature sensor whose samples
 * walk up a ramp and start over. One instance per enabled devicetree node.
 */
#define DT_DRV_COMPAT vnd_ramp_temp

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/spinlock.h>

struct ramp_temp_config {
	int32_t start_mc;
	int32_t step_mc;
	int32_t steps;
};

struct ramp_temp_data {
	struct k_spinlock lock;
	int32_t index;
	int32_t sample_mc;
	bool has_sample;
};

static int ramp_temp_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct ramp_temp_config *cfg = dev->config;
	struct ramp_temp_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	k_spinlock_key_t key = k_spin_lock(&data->lock);

	data->sample_mc = cfg->start_mc + data->index * cfg->step_mc;
	data->index = (data->index + 1) % cfg->steps;
	data->has_sample = true;
	k_spin_unlock(&data->lock, key);
	return 0;
}

static int ramp_temp_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
	struct ramp_temp_data *data = dev->data;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	k_spinlock_key_t key = k_spin_lock(&data->lock);
	int32_t mc = data->sample_mc;
	bool has_sample = data->has_sample;

	k_spin_unlock(&data->lock, key);
	if (!has_sample) {
		return -ENODATA;
	}

	/* sensor_value is whole units plus millionths, both with the same sign. */
	val->val1 = mc / 1000;
	val->val2 = (mc % 1000) * 1000;
	return 0;
}

static int ramp_temp_init(const struct device *dev)
{
	const struct ramp_temp_config *cfg = dev->config;

	/* A failed init makes device_is_ready() false instead of dividing by zero later. */
	return cfg->steps >= 1 ? 0 : -EINVAL;
}

static DEVICE_API(sensor, ramp_temp_api) = {
	.sample_fetch = ramp_temp_sample_fetch,
	.channel_get = ramp_temp_channel_get,
};

#define RAMP_TEMP_DEFINE(inst)                                                                                 \
	static struct ramp_temp_data ramp_temp_data_##inst;                                                    \
	static const struct ramp_temp_config ramp_temp_config_##inst = {                                       \
		.start_mc = DT_INST_PROP(inst, start_millicelsius),                                            \
		.step_mc = DT_INST_PROP(inst, step_millicelsius),                                              \
		.steps = DT_INST_PROP(inst, ramp_steps),                                                       \
	};                                                                                                     \
	DEVICE_DT_INST_DEFINE(inst, ramp_temp_init, NULL, &ramp_temp_data_##inst, &ramp_temp_config_##inst,   \
			      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &ramp_temp_api);

DT_INST_FOREACH_STATUS_OKAY(RAMP_TEMP_DEFINE)
