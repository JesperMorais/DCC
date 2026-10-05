#include "channel.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

/* Reads <dir>/<name> as a long (integer) or a double (!integer). */
static int read_attr(const char *dir, const char *name, bool integer, double *out) {
    char path[PATH_MAX], buf[64];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    if (read_small(path, buf, sizeof buf) != 0) return -1;
    char *s = trim(buf), *end;
    errno = 0;
    double v = integer ? (double)strtol(s, &end, 10) : strtod(s, &end);
    if (*s == '\0' || *end || errno || !isfinite(v)) return -1;
    *out = v;
    return 0;
}

int channel_read_temp(const char *dir, double *out) {
    double raw, scale, offset = 0;
    if (read_attr(dir, "in_temp_raw", true, &raw) != 0) return -1;
    if (read_attr(dir, "in_temp_scale", false, &scale) != 0) return -1;
    /* The offset is optional: a missing file means 0, a broken one is an error. */
    if (read_attr(dir, "in_temp_offset", true, &offset) != 0 && errno != ENOENT) return -1;
    *out = (raw + offset) * scale;
    return 0;
}
