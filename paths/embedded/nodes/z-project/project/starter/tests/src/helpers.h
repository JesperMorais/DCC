/*
 * Shared by the m3 to m5 suites: run a shell command through the dummy
 * backend and pick numbers out of what it printed.
 */
#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>
#include <zephyr/ztest.h>

#if !defined(CONFIG_SENSOR_NODE_SAMPLE_MS) || !defined(CONFIG_SENSOR_NODE_REPORT_MS)
#error "Finish milestone 1 first: the Kconfig options SENSOR_NODE_SAMPLE_MS and SENSOR_NODE_REPORT_MS are missing"
#endif

#define SAMPLE_MS CONFIG_SENSOR_NODE_SAMPLE_MS
#define REPORT_MS CONFIG_SENSOR_NODE_REPORT_MS

/* Suite setup: give the shell thread and the app's threads time to start. */
static inline void *settle(void)
{
	k_msleep(100);
	return NULL;
}

/* Runs cmd, copies what it printed into out and returns its result (0 = ok). */
static inline int run_cmd(const char *cmd, char *out, size_t len)
{
	const struct shell *sh = shell_backend_dummy_get_ptr();
	size_t size;

	(void)shell_backend_dummy_get_output(sh, &size); /* drop older output */
	int ret = shell_execute_cmd(sh, cmd);
	const char *text = shell_backend_dummy_get_output(sh, &size);

	snprintf(out, len, "%s", text);
	return ret;
}

/* Parses "20.750" or "-1.250" (exactly three decimals) or "12" into thousandths. */
static inline bool parse_milli(const char *s, long long *value)
{
	bool negative = false;
	long long whole = 0;
	long long frac = 0;
	int digits = 0;

	if (*s == '-') {
		negative = true;
		s++;
	}
	for (; *s >= '0' && *s <= '9'; s++, digits++) {
		whole = whole * 10 + (*s - '0');
	}
	if (digits == 0) {
		return false;
	}
	if (*s == '.') {
		s++;
		for (digits = 0; *s >= '0' && *s <= '9'; s++, digits++) {
			frac = frac * 10 + (*s - '0');
		}
		if (digits != 3) {
			return false;
		}
	}
	*value = (negative ? -1 : 1) * (whole * 1000 + frac);
	return true;
}

/* Finds key (e.g. "min=") in out and parses the number after it. */
static inline bool field_milli(const char *out, const char *key, long long *value)
{
	const char *at = strstr(out, key);

	return at != NULL && parse_milli(at + strlen(key), value);
}

/* Same, for a whole number such as "samples=12". */
static inline bool field_count(const char *out, const char *key, long long *value)
{
	const char *at = strstr(out, key);
	long long milli;

	if (at == NULL || !parse_milli(at + strlen(key), &milli) || milli % 1000 != 0) {
		return false;
	}
	*value = milli / 1000;
	return true;
}

/* run_cmd("node stats") that must succeed and contain key. */
static inline long long stats_count(const char *key)
{
	char out[256];
	long long value = -1;

	zassert_ok(run_cmd("node stats", out, sizeof(out)), "`node stats` failed. It printed: %s", out);
	zassert_true(field_count(out, key, &value), "`node stats` printed \"%s\", with no whole number after %s", out, key);
	return value;
}

#endif /* HELPERS_H */
