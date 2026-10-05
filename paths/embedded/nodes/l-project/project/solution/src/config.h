#ifndef CONFIG_H
#define CONFIG_H

#include <limits.h>
#include <stddef.h>

struct config {
    char fifo[PATH_MAX];
    char sysfs[PATH_MAX];
    char log_dir[PATH_MAX];
    char control[PATH_MAX]; /* "" = no control socket */
    int period_ms;
    int rotate_lines;       /* 0 = never rotate */
    int keep;
};

/* Fills *cfg from the file at path. Returns 0, or -1 with a message like
 * "sensord.conf:3: expected key = value" in err. *cfg is untouched on error. */
int config_load(const char *path, struct config *cfg, char *err, size_t errlen);

#endif
