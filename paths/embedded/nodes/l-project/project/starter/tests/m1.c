/* Milestone 1: the config file and one reading of the temperature channel. */
#include <stdlib.h>
#include <string.h>

#include "harness.h"

static bool stderr_has(const char *needle) {
    char *err = read_file("err.txt");
    bool found = err && strstr(err, needle);
    free(err);
    return found;
}

static bool stdout_is(const char *expected) {
    char *out = read_file("out.txt");
    bool same = out && strcmp(out, expected) == 0;
    if (!same) printf("#   stdout was \"%s\", expected \"%s\"\n", out ? out : "(nothing)", expected);
    free(out);
    return same;
}

static void no_arguments_prints_usage(void) {
    CHECK(run_once(NULL, NULL, 2000) == 2);
    char *err = read_file("err.txt");
    CHECKM(err && strncmp(err, "usage: ", 7) == 0, "stderr should start with \"usage: \"");
    free(err);
}

static void unknown_flag_prints_usage(void) {
    write_conf(100, NULL);
    CHECK(run_once("--verbose", "sensord.conf", 2000) == 2);
    CHECK(stderr_has("usage: "));
}

static void once_prints_the_temperature(void) {
    write_conf(100, NULL);
    fake_temp("iio", "23500", "0.001", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 0);
    CHECK(stdout_is("temp=23.500\n"));
}

static void once_applies_offset_and_scale(void) {
    write_conf(100, NULL);
    fake_temp("iio", "-120", "0.25", "40"); /* (-120 + 40) * 0.25 = -20 */
    CHECK(run_once("--once", "sensord.conf", 2000) == 0);
    CHECK(stdout_is("temp=-20.000\n"));
}

static void once_does_not_need_the_fifo_or_logs(void) {
    write_conf(100, NULL);
    fake_temp("iio", "1", "1", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 0);
    CHECK(stdout_is("temp=1.000\n"));
    CHECKM(!exists("sensor.fifo") && !exists("logs"), "--once must not create the FIFO or the log dir");
}

static void once_fails_on_a_broken_channel(void) {
    write_conf(100, NULL);
    fake_temp("iio", "warm", "0.001", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECK(stdout_is(""));
    CHECKM(stderr_has("iio"), "the error should name the channel directory");
    fake_temp("iio", NULL, "0.001", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
}

static void config_comments_spaces_and_defaults(void) {
    write_file("sensord.conf",
               "# only the required keys\n"
               "\n"
               "   fifo=sensor.fifo   \n"
               "\tsysfs   =   iio\n"
               "log_dir = logs\n");
    fake_temp("iio", "500", "0.1", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 0);
    CHECK(stdout_is("temp=50.000\n"));
}

static void missing_config_file(void) {
    CHECK(run_once("--once", "nope.conf", 2000) == 1);
    CHECKM(stderr_has("nope.conf"), "the error should name the file");
}

static void bad_line_is_reported_with_its_number(void) {
    write_file("sensord.conf", "fifo = sensor.fifo\n# fine\nthis line has no equals sign\nsysfs = iio\nlog_dir = logs\n");
    fake_temp("iio", "1", "1", NULL);
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sensord.conf:3:"), "expected \"sensord.conf:3:\" in the error");
}

static void unknown_key_and_bad_values(void) {
    fake_temp("iio", "1", "1", NULL);
    write_conf(100, "colour = blue\n");
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sensord.conf:6:"), "unknown key on line 6");

    write_conf(5, NULL); /* below 10 */
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sensord.conf:5:"), "period_ms 5 is out of range");

    write_conf(100, "keep = lots\n");
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sensord.conf:6:"), "keep must be a number");

    write_conf(100, "rotate_lines = 10x\n");
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sensord.conf:6:"), "rotate_lines must be a whole number");
}

static void missing_required_key(void) {
    fake_temp("iio", "1", "1", NULL);
    write_file("sensord.conf", "fifo = sensor.fifo\nlog_dir = logs\n");
    CHECK(run_once("--once", "sensord.conf", 2000) == 1);
    CHECKM(stderr_has("sysfs"), "the error should name the missing key");
}

void m1_tests(void) {
    RUN(no_arguments_prints_usage);
    RUN(unknown_flag_prints_usage);
    RUN(once_prints_the_temperature);
    RUN(once_applies_offset_and_scale);
    RUN(once_does_not_need_the_fifo_or_logs);
    RUN(once_fails_on_a_broken_channel);
    RUN(config_comments_spaces_and_defaults);
    RUN(missing_config_file);
    RUN(bad_line_is_reported_with_its_number);
    RUN(unknown_key_and_bad_values);
    RUN(missing_required_key);
}
