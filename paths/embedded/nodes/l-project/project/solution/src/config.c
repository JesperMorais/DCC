#include "config.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* A whole-string integer in [min, max]. */
static bool parse_int(const char *s, int min, int max, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (*s == '\0' || *end || errno || v < min || v > max) return false;
    *out = (int)v;
    return true;
}

static bool set_path(char *dst, const char *value) {
    if (!*value || strlen(value) >= PATH_MAX) return false;
    strcpy(dst, value);
    return true;
}

int config_load(const char *path, struct config *cfg, char *err, size_t errlen) {
    FILE *f = fopen(path, "r");
    if (!f) {
        snprintf(err, errlen, "%s: %s", path, strerror(errno));
        return -1;
    }
    struct config c = { .period_ms = 1000, .rotate_lines = 0, .keep = 3 };
    char line[1024];
    int lineno = 0, rc = 0;
    while (rc == 0 && fgets(line, sizeof line, f)) {
        lineno++;
        char *s = trim(line);
        if (*s == '\0' || *s == '#') continue;
        char *eq = strchr(s, '=');
        if (!eq) {
            snprintf(err, errlen, "%s:%d: expected key = value", path, lineno);
            rc = -1;
            break;
        }
        *eq = '\0';
        char *key = trim(s), *value = trim(eq + 1);
        bool ok;
        if (strcmp(key, "fifo") == 0) ok = set_path(c.fifo, value);
        else if (strcmp(key, "sysfs") == 0) ok = set_path(c.sysfs, value);
        else if (strcmp(key, "log_dir") == 0) ok = set_path(c.log_dir, value);
        else if (strcmp(key, "control") == 0) ok = set_path(c.control, value);
        else if (strcmp(key, "period_ms") == 0) ok = parse_int(value, 10, 60000, &c.period_ms);
        else if (strcmp(key, "rotate_lines") == 0) ok = parse_int(value, 0, 1000000, &c.rotate_lines);
        else if (strcmp(key, "keep") == 0) ok = parse_int(value, 1, 9, &c.keep);
        else {
            snprintf(err, errlen, "%s:%d: unknown key \"%s\"", path, lineno, key);
            rc = -1;
            break;
        }
        if (!ok) {
            snprintf(err, errlen, "%s:%d: bad value \"%s\" for %s", path, lineno, value, key);
            rc = -1;
        }
    }
    fclose(f);
    if (rc) return rc;
    const char *missing = !*c.fifo ? "fifo" : !*c.sysfs ? "sysfs" : !*c.log_dir ? "log_dir" : NULL;
    if (missing) {
        snprintf(err, errlen, "%s: missing %s", path, missing);
        return -1;
    }
    *cfg = c;
    return 0;
}
