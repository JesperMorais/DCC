/* Milestone 1: the app's own Kconfig options exist with the right defaults. */
#include <zephyr/ztest.h>

ZTEST(m1_kconfig, test_sample_period_option)
{
#ifdef CONFIG_SENSOR_NODE_SAMPLE_MS
	zassert_equal(CONFIG_SENSOR_NODE_SAMPLE_MS, 100, "SENSOR_NODE_SAMPLE_MS should default to 100, got %d",
		      CONFIG_SENSOR_NODE_SAMPLE_MS);
#else
	zassert_unreachable("Kconfig has no SENSOR_NODE_SAMPLE_MS option (int, default 100)");
#endif
}

ZTEST(m1_kconfig, test_report_period_option)
{
#ifdef CONFIG_SENSOR_NODE_REPORT_MS
	zassert_equal(CONFIG_SENSOR_NODE_REPORT_MS, 1000, "SENSOR_NODE_REPORT_MS should default to 1000, got %d",
		      CONFIG_SENSOR_NODE_REPORT_MS);
#else
	zassert_unreachable("Kconfig has no SENSOR_NODE_REPORT_MS option (int, default 1000)");
#endif
}

ZTEST(m1_kconfig, test_queue_length_option)
{
#ifdef CONFIG_SENSOR_NODE_QUEUE_LEN
	zassert_equal(CONFIG_SENSOR_NODE_QUEUE_LEN, 8, "SENSOR_NODE_QUEUE_LEN should default to 8, got %d",
		      CONFIG_SENSOR_NODE_QUEUE_LEN);
#else
	zassert_unreachable("Kconfig has no SENSOR_NODE_QUEUE_LEN option (int, default 8)");
#endif
}

ZTEST_SUITE(m1_kconfig, NULL, NULL, NULL, NULL, NULL);
