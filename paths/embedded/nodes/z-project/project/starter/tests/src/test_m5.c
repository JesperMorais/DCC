/* Milestone 5: `node read` and `node period <ms>`. */
#include "helpers.h"

static void restore_period(void *fixture)
{
	char out[128];

	ARG_UNUSED(fixture);
	(void)run_cmd("node period " STRINGIFY(CONFIG_SENSOR_NODE_REPORT_MS), out, sizeof(out));
}

ZTEST(m5_controls, test_read_prints_a_ramp_value)
{
	char out[256];
	long long value = 0;

	zassert_ok(run_cmd("node read", out, sizeof(out)), "`node read` failed. It printed: %s", out);
	zassert_true(field_milli(out, "ambient: ", &value), "expected \"ambient: 20.500 C\", got \"%s\"", out);
	zassert_true(value == 20000 || value == 20500 || value == 21000 || value == 21500,
		     "%lld isn't on the ambient ramp (20.000 to 21.500)", value);
	zassert_not_null(strstr(out, " C"), "the reading should end with the unit, \" C\": \"%s\"", out);
}

ZTEST(m5_controls, test_period_changes_the_report_rate)
{
	char out[256];

	zassert_ok(run_cmd("node period 200", out, sizeof(out)), "`node period 200` failed. It printed: %s", out);
	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed");
	k_msleep(1100);

	long long reports = stats_count("reports=");

	zassert_true(reports >= 4 && reports <= 6, "1.1 s at a 200 ms period should give about 5 reports, got %lld",
		     reports);
}

ZTEST(m5_controls, test_bad_periods_are_rejected)
{
	static const char *const bad[] = {
		"node period abc", "node period 70000", "node period 200x",
		"node period 5",   "node period -100",  "node period",
	};
	char out[256];

	for (size_t i = 0; i < ARRAY_SIZE(bad); i++) {
		zassert_not_equal(run_cmd(bad[i], out, sizeof(out)), 0, "`%s` should fail, but returned 0", bad[i]);
	}

	/* And the period is still the default one. */
	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed");
	k_msleep(REPORT_MS + REPORT_MS / 2);
	long long reports = stats_count("reports=");

	zassert_true(reports >= 1 && reports <= 2,
		     "after only bad commands the period should still be %d ms: got %lld reports in 1.5 periods",
		     REPORT_MS, reports);
}

ZTEST_SUITE(m5_controls, NULL, settle, NULL, restore_period, NULL);
