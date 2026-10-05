/*
 * Milestone 3: sampling, the queue and the statistics, seen through
 * `node stats` and `node reset`. The ambient sensor in tests/app.overlay
 * ramps 20.000, 20.500, 21.000, 21.500.
 */
#include "helpers.h"

ZTEST(m3_pipeline, test_stats_prints_every_field)
{
	static const char *const keys[] = {"samples=", "min=", "max=", "mean=", "dropped="};
	char out[256];

	zassert_ok(run_cmd("node stats", out, sizeof(out)), "`node stats` failed. It printed: %s", out);
	for (size_t i = 0; i < ARRAY_SIZE(keys); i++) {
		zassert_not_null(strstr(out, keys[i]), "`node stats` printed \"%s\", which has no %s", out, keys[i]);
	}
}

ZTEST(m3_pipeline, test_reset_starts_from_zero)
{
	char out[256];
	long long samples = -1;

	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed. It printed: %s", out);
	zassert_ok(run_cmd("node stats", out, sizeof(out)), "`node stats` failed. It printed: %s", out);
	zassert_true(field_count(out, "samples=", &samples), "no samples= in \"%s\"", out);
	zassert_true(samples <= 1, "right after a reset samples should be 0, got %lld", samples);
	if (samples == 0) {
		zassert_not_null(strstr(out, "min=n/a"), "with no samples, min should be n/a: \"%s\"", out);
		zassert_not_null(strstr(out, "mean=n/a"), "with no samples, mean should be n/a: \"%s\"", out);
	}
}

ZTEST(m3_pipeline, test_ten_periods_give_ten_samples)
{
	char out[256];
	long long samples = -1, min = 0, max = 0, mean = 0, dropped = -1;

	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed");
	k_msleep(10 * SAMPLE_MS);
	zassert_ok(run_cmd("node stats", out, sizeof(out)), "`node stats` failed");

	zassert_true(field_count(out, "samples=", &samples), "no samples= in \"%s\"", out);
	zassert_true(samples >= 9 && samples <= 11, "10 sample periods should give about 10 samples, got %lld",
		     samples);
	zassert_true(field_milli(out, "min=", &min), "min= should be a number with 3 decimals: \"%s\"", out);
	zassert_true(field_milli(out, "max=", &max), "max= should be a number with 3 decimals: \"%s\"", out);
	zassert_true(field_milli(out, "mean=", &mean), "mean= should be a number with 3 decimals: \"%s\"", out);
	zassert_equal(min, 20000, "the ramp's lowest value is 20.000: \"%s\"", out);
	zassert_equal(max, 21500, "the ramp's highest value is 21.500: \"%s\"", out);
	zassert_true(mean >= 20500 && mean <= 21000, "the mean of the ramp should be close to 20.750: \"%s\"", out);
	zassert_true(field_count(out, "dropped=", &dropped), "no dropped= in \"%s\"", out);
	zassert_equal(dropped, 0, "nothing should be dropped under normal load: \"%s\"", out);
}

ZTEST_SUITE(m3_pipeline, NULL, settle, NULL, NULL, NULL);
