#ifndef STATS_H
#define STATS_H

#include <stddef.h>
#include <stdint.h>

/* Everything `node stats` and the report show, copied out in one piece. */
struct stats {
	uint32_t samples;
	int32_t min_mc;
	int32_t max_mc;
	int64_t sum_mc;
	uint32_t dropped;
	uint32_t reports;
};

void stats_add(int32_t milli_c);
void stats_count_drop(void);
void stats_count_report(void);
void stats_reset(void);
void stats_get(struct stats *out);

/* "21.250", "-0.500": millidegrees as text with three decimals. */
void format_milli(char *buf, size_t len, int64_t milli);

#endif /* STATS_H */
