/*
 * The statistics, shared by the processing thread, the report work item and
 * the shell. Every access goes through one mutex, so a reader always gets a
 * consistent copy and never sees samples updated without the sum.
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "stats.h"

static struct stats stats;
K_MUTEX_DEFINE(stats_lock);

void stats_add(int32_t milli_c)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	if (stats.samples == 0 || milli_c < stats.min_mc) {
		stats.min_mc = milli_c;
	}
	if (stats.samples == 0 || milli_c > stats.max_mc) {
		stats.max_mc = milli_c;
	}
	stats.sum_mc += milli_c;
	stats.samples++;
	k_mutex_unlock(&stats_lock);
}

void stats_count_drop(void)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	stats.dropped++;
	k_mutex_unlock(&stats_lock);
}

void stats_count_report(void)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	stats.reports++;
	k_mutex_unlock(&stats_lock);
}

void stats_reset(void)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	stats = (struct stats){0};
	k_mutex_unlock(&stats_lock);
}

void stats_get(struct stats *out)
{
	k_mutex_lock(&stats_lock, K_FOREVER);
	*out = stats;
	k_mutex_unlock(&stats_lock);
}

void format_milli(char *buf, size_t len, int64_t milli)
{
	const char *sign = milli < 0 ? "-" : "";
	int64_t abs_milli = milli < 0 ? -milli : milli;

	snprintk(buf, len, "%s%d.%03d", sign, (int)(abs_milli / 1000), (int)(abs_milli % 1000));
}
