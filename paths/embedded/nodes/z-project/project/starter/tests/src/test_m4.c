/* Milestone 4: the periodic report, counted by `node stats` as reports=. */
#include "helpers.h"

ZTEST(m4_reports, test_reports_arrive_every_period)
{
	char out[256];

	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed");
	zassert_true(stats_count("reports=") <= 1, "a reset should set reports back to 0");

	k_msleep(5 * REPORT_MS + REPORT_MS / 2);
	long long reports = stats_count("reports=");

	zassert_true(reports >= 5 && reports <= 6, "5.5 report periods should give 5 or 6 reports, got %lld", reports);
}

ZTEST(m4_reports, test_reports_leave_the_stats_alone)
{
	char out[256];

	zassert_ok(run_cmd("node reset", out, sizeof(out)), "`node reset` failed");
	k_msleep(2 * REPORT_MS + REPORT_MS / 2);

	long long samples = stats_count("samples=");
	long long want = (2 * REPORT_MS + REPORT_MS / 2) / SAMPLE_MS;

	zassert_true(stats_count("reports=") >= 2, "2.5 report periods should give at least 2 reports");
	zassert_true(samples >= want - 2 && samples <= want + 2,
		     "a report must not reset the statistics: expected about %lld samples since the reset, got %lld",
		     want, samples);
}

ZTEST_SUITE(m4_reports, NULL, settle, NULL, NULL, NULL);
