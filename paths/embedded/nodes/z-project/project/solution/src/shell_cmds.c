/* The "node" shell command: node stats | reset | read | period <ms>. */
#include <errno.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "reporter.h"
#include "sampler.h"
#include "stats.h"

static int cmd_stats(const struct shell *sh, size_t argc, char **argv)
{
	struct stats s;
	char min[16] = "n/a", max[16] = "n/a", mean[16] = "n/a";

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	stats_get(&s);
	if (s.samples > 0) {
		format_milli(min, sizeof(min), s.min_mc);
		format_milli(max, sizeof(max), s.max_mc);
		format_milli(mean, sizeof(mean), s.sum_mc / s.samples);
	}
	shell_print(sh, "samples=%u min=%s max=%s mean=%s dropped=%u reports=%u", (unsigned int)s.samples, min, max,
		    mean, (unsigned int)s.dropped, (unsigned int)s.reports);
	return 0;
}

static int cmd_reset(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	stats_reset();
	shell_print(sh, "statistics cleared");
	return 0;
}

static int cmd_read(const struct shell *sh, size_t argc, char **argv)
{
	int32_t milli_c;
	char text[16];

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int err = sampler_read(&milli_c);

	if (err != 0) {
		shell_error(sh, "reading the sensor failed: %d", err);
		return err;
	}
	format_milli(text, sizeof(text), milli_c);
	shell_print(sh, "ambient: %s C", text);
	return 0;
}

static int cmd_period(const struct shell *sh, size_t argc, char **argv)
{
	char *end;
	long ms;

	ARG_UNUSED(argc);

	ms = strtol(argv[1], &end, 10);
	if (end == argv[1] || *end != '\0' || ms < REPORTER_MIN_MS || ms > REPORTER_MAX_MS) {
		shell_error(sh, "period must be a whole number of ms from %d to %d", REPORTER_MIN_MS, REPORTER_MAX_MS);
		return -EINVAL;
	}

	(void)reporter_set_period((uint32_t)ms);
	shell_print(sh, "report period %ld ms", ms);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_node,
	SHELL_CMD(stats, NULL, "Show the statistics since the last reset.", cmd_stats),
	SHELL_CMD(reset, NULL, "Clear the statistics.", cmd_reset),
	SHELL_CMD(read, NULL, "Read the ambient sensor now.", cmd_read),
	SHELL_CMD_ARG(period, NULL, "Set the report period: period <ms>", cmd_period, 2, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(node, &sub_node, "Sensor node commands", NULL);
