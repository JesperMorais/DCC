/* sensord: a sensor logging daemon.
 *
 * Usage: sensord [--once] <config>
 */
#include <stdio.h>
#include <string.h>

#include "channel.h"
#include "config.h"
#include "daemon.h"

static int once(const char *config_path) {
    struct config cfg;
    char err[512];
    if (config_load(config_path, &cfg, err, sizeof err) != 0) {
        fprintf(stderr, "sensord: %s\n", err);
        return 1;
    }
    double t;
    if (channel_read_temp(cfg.sysfs, &t) != 0) {
        fprintf(stderr, "sensord: cannot read the temperature in %s\n", cfg.sysfs);
        return 1;
    }
    printf("temp=%.3f\n", t);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && argv[1][0] != '-') return daemon_run(argv[1]);
    if (argc == 3 && strcmp(argv[1], "--once") == 0) return once(argv[2]);
    fprintf(stderr, "usage: sensord [--once] <config>\n");
    return 2;
}
