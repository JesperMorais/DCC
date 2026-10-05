/*
 * Milestone 2: the vnd,ramp-temp driver, through the standard sensor API.
 * Uses the probe and fresh nodes from tests/app.overlay. If this fails to
 * link with "undefined reference to __device_dts_ord_...", no driver
 * instance exists for those nodes yet.
 */
#include <errno.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/ztest.h>

static const struct device *const ambient = DEVICE_DT_GET(DT_ALIAS(ambient_temp));
static const struct device *const probe = DEVICE_DT_GET(DT_NODELABEL(probe));
static const struct device *const fresh = DEVICE_DT_GET(DT_NODELABEL(fresh));

static int32_t read_milli(const struct device *dev, struct sensor_value *val)
{
	zassert_ok(sensor_sample_fetch(dev), "sensor_sample_fetch(%s) failed", dev->name);
	zassert_ok(sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, val),
		   "sensor_channel_get(%s, SENSOR_CHAN_AMBIENT_TEMP) failed", dev->name);
	return val->val1 * 1000 + val->val2 / 1000;
}

ZTEST(m2_driver, test_every_instance_is_ready)
{
	zassert_true(device_is_ready(ambient), "the ambient-temp device isn't ready");
	zassert_true(device_is_ready(probe), "the probe device isn't ready");
	zassert_true(device_is_ready(fresh), "the fresh device isn't ready");
}

ZTEST(m2_driver, test_first_samples_follow_the_devicetree)
{
	struct sensor_value val;

	/* start 1250, step 1000, 2 steps */
	zassert_equal(read_milli(fresh, &val), 1250, "the first sample should be start-millicelsius (1.250)");
	zassert_equal(val.val1, 1, "1.250 C is val1 = 1, got %d", val.val1);
	zassert_equal(val.val2, 250000, "1.250 C is val2 = 250000 (millionths), got %d", val.val2);
	zassert_equal(read_milli(fresh, &val), 2250, "the second sample should be start + step (2.250)");
	zassert_equal(read_milli(fresh, &val), 1250, "after ramp-steps samples the ramp starts over (1.250)");
}

ZTEST(m2_driver, test_ramp_steps_and_wraps)
{
	struct sensor_value val;
	int32_t prev = read_milli(probe, &val);

	/* start 5000, step 250, 3 steps: 5.000, 5.250, 5.500, 5.000, ... */
	zassert_true(prev == 5000 || prev == 5250 || prev == 5500, "%d isn't on the probe's ramp", prev);
	for (int i = 0; i < 7; i++) {
		int32_t next = read_milli(probe, &val);
		int32_t want = prev == 5500 ? 5000 : prev + 250;

		zassert_equal(next, want, "after %d the probe should give %d, got %d", prev, want, next);
		prev = next;
	}
}

ZTEST(m2_driver, test_other_channels_are_not_supported)
{
	struct sensor_value val;

	zassert_equal(sensor_sample_fetch_chan(probe, SENSOR_CHAN_ACCEL_X), -ENOTSUP,
		      "fetching an accelerometer channel should return -ENOTSUP");
	zassert_ok(sensor_sample_fetch(probe), "sensor_sample_fetch(probe) failed");
	zassert_equal(sensor_channel_get(probe, SENSOR_CHAN_ACCEL_X, &val), -ENOTSUP,
		      "reading an accelerometer channel should return -ENOTSUP");
}

ZTEST_SUITE(m2_driver, NULL, NULL, NULL, NULL, NULL);
