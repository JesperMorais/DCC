/*
 * Driver for "vnd,ramp-temp" (milestone 2). The plumbing is here: the
 * compatible, the per-instance config filled from the devicetree, the API
 * struct and the macro that creates one device per enabled node. What the
 * sensor actually does is yours: fill in the TODOs.
 *
 * Until then every fetch and get fails with -ENOSYS, so the m2 tests fail.
 *
 * Until app.overlay has a vnd,ramp-temp node, the macro at the bottom
 * creates no device, and gcc warns that ramp_temp_init is unused. That's
 * expected: the warning goes away when you add the node in milestone 2.
 */
#define DT_DRV_COMPAT vnd_ramp_temp

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

/* Read-only, one per instance, straight from the node's properties. */
struct ramp_temp_config {
	int32_t start_mc;
	int32_t step_mc;
	int32_t steps;
};

/* Mutable, one per instance. */
struct ramp_temp_data {
	/* TODO: where is this instance on its ramp, and what was its last sample? */
};

static int ramp_temp_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	/* TODO: check the channel, take the next value of the ramp, advance and wrap. */
	ARG_UNUSED(dev);
	ARG_UNUSED(chan);
	return -ENOSYS;
}

static int ramp_temp_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
	/* TODO: check the channel, turn the last sample into a struct sensor_value. */
	ARG_UNUSED(dev);
	ARG_UNUSED(chan);
	ARG_UNUSED(val);
	return -ENOSYS;
}

static int ramp_temp_init(const struct device *dev)
{
	/* TODO: return an error for a ramp that can't work, so device_is_ready() is false. */
	ARG_UNUSED(dev);
	return 0;
}

static DEVICE_API(sensor, ramp_temp_api) = {
	.sample_fetch = ramp_temp_sample_fetch,
	.channel_get = ramp_temp_channel_get,
};

/* Runs once per enabled vnd,ramp-temp node; inst is 0, 1, 2, ... */
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
