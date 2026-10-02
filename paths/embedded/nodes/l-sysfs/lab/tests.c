#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* ---- fixture helpers: build a fake sysfs tree in the current directory ---- */

static void mkdirs(const char *path) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof tmp, "%s", path);
    for (char *p = tmp + 1; *p; p++)
        if (*p == '/') { *p = '\0'; mkdir(tmp, 0755); *p = '/'; }
    mkdir(tmp, 0755);
}

/* Write `content` to root/rel, creating parent directories. */
static void put(const char *root, const char *rel, const char *content) {
    char path[PATH_MAX];
    snprintf(path, sizeof path, "%s/%s", root, rel);
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", path);
    *strrchr(dir, '/') = '\0';
    mkdirs(dir);
    FILE *f = fopen(path, "w");
    if (f) { fputs(content, f); fclose(f); }
}

static int open_fd_count(void) {
    int n = 0;
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}

static void sleep_ms(long ms) {
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

/* A "slow" attribute: a FIFO whose writer delivers the value in two chunks,
 * so a single read() only sees the first part. */
static void *slow_writer(void *path) {
    int fd = -1;
    for (int tries = 0; fd < 0 && tries < 60; tries++) {   /* wait up to ~300 ms for a reader */
        fd = open(path, O_WRONLY | O_NONBLOCK);
        if (fd < 0) sleep_ms(5);
    }
    if (fd < 0) return NULL;
    if (write(fd, "-1", 2) < 0) { /* ignore */ }
    sleep_ms(40);
    if (write(fd, "2500\n", 5) < 0) { /* ignore */ }
    close(fd);
    return NULL;
}

/* ---- tests ---- */

TEST(reads_millidegrees_and_ignores_the_newline) {
    put("t1", "sys/class/thermal/thermal_zone0/temp", "45123\n");
    put("t1", "sys/class/hwmon/hwmon0/temp1_input", "  -5000 \n");
    long v = 0;
    EXPECT_EQ(sysfs_read_long("t1/sys/class/thermal/thermal_zone0/temp", &v), 0);
    EXPECT_EQ(v, 45123);
    EXPECT_EQ(sysfs_read_long("t1/sys/class/hwmon/hwmon0/temp1_input", &v), 0);
    EXPECT_EQ(v, -5000);
}

TEST(missing_files_return_minus_enoent) {
    long v = 7;
    char buf[16];
    EXPECT_EQ(sysfs_read_long("t2/sys/class/thermal/thermal_zone9/temp", &v), -ENOENT);
    EXPECT_EQ(v, 7);   /* untouched on error */
    EXPECT_EQ(sysfs_read_string("t2/nope/type", buf, sizeof buf), -ENOENT);
}

TEST(empty_or_garbage_values_return_minus_einval) {
    put("t3", "empty", "");
    put("t3", "blank", "\n");
    put("t3", "word", "hot\n");
    put("t3", "junk", "42C\n");
    long v = 7;
    EXPECT_EQ(sysfs_read_long("t3/empty", &v), -EINVAL);
    EXPECT_EQ(sysfs_read_long("t3/blank", &v), -EINVAL);
    EXPECT_EQ(sysfs_read_long("t3/word", &v), -EINVAL);
    EXPECT_EQ(sysfs_read_long("t3/junk", &v), -EINVAL);
    EXPECT_EQ(v, 7);
}

TEST(read_string_strips_trailing_whitespace_and_checks_size) {
    put("t4", "type", "x86_pkg_temp\n");
    char buf[32];
    EXPECT_EQ(sysfs_read_string("t4/type", buf, sizeof buf), 12);
    EXPECT_STR_EQ(buf, "x86_pkg_temp");
    char tiny[8];   /* "x86_pkg_temp" can't fit in 7 chars + NUL */
    EXPECT_EQ(sysfs_read_string("t4/type", tiny, sizeof tiny), -ERANGE);
}

TEST(finds_the_zone_by_type) {
    put("t5", "sys/class/thermal/thermal_zone0/type", "acpitz\n");
    put("t5", "sys/class/thermal/thermal_zone0/temp", "27800\n");
    put("t5", "sys/class/thermal/thermal_zone12/type", "cpu-thermal\n");
    put("t5", "sys/class/thermal/thermal_zone12/temp", "71250\n");
    put("t5", "sys/class/thermal/thermal_zone3/type", "cpu-thermal\n");
    put("t5", "sys/class/thermal/thermal_zone3/temp", "68000\n");
    put("t5", "sys/class/thermal/cooling_device0/type", "cpu-thermal\n");
    long t = 0;
    /* zones 3 and 12 both match: the lowest number wins, whatever order readdir() uses */
    EXPECT_EQ(thermal_find_zone("t5", "cpu-thermal", &t), 3);
    EXPECT_EQ(t, 68000);
    EXPECT_EQ(thermal_find_zone("t5", "acpitz", &t), 0);
    EXPECT_EQ(t, 27800);
    EXPECT_EQ(thermal_find_zone("t5", "gpu-thermal", &t), -ENOENT);
    EXPECT_EQ(thermal_find_zone("no-such-root", "acpitz", &t), -ENOENT);
}

TEST(handles_values_that_arrive_in_several_reads) {
    mkdirs("t6");
    EXPECT_EQ(mkfifo("t6/temp", 0644), 0);
    signal(SIGPIPE, SIG_IGN);   /* if the reader gives up early, the writer just gets EPIPE */
    pthread_t w;
    pthread_create(&w, NULL, slow_writer, "t6/temp");
    long v = 0;
    int rc = sysfs_read_long("t6/temp", &v);
    pthread_join(w, NULL);
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(v, -12500);   /* "-1" + "2500\n": one read() would only see "-1" */
}

TEST(never_leaks_a_file_descriptor) {
    put("t7", "sys/class/thermal/thermal_zone0/type", "acpitz\n");
    put("t7", "sys/class/thermal/thermal_zone0/temp", "bogus\n");
    put("t7", "sys/class/thermal/thermal_zone1/type", "soc\n");
    put("t7", "sys/class/thermal/thermal_zone1/temp", "50000\n");
    put("t7", "long", "this value is far too long for the buffer\n");
    int before = open_fd_count();
    long v;
    char buf[8];
    for (int i = 0; i < 300; i++) {
        sysfs_read_long("t7/sys/class/thermal/thermal_zone1/temp", &v);   /* success */
        sysfs_read_long("t7/sys/class/thermal/thermal_zone0/temp", &v);   /* -EINVAL */
        sysfs_read_string("t7/long", buf, sizeof buf);                     /* -ERANGE */
        thermal_find_zone("t7", "soc", &v);
        thermal_find_zone("t7", "acpitz", &v);                             /* temp is bogus */
    }
    EXPECT_EQ(thermal_find_zone("t7", "acpitz", &v), -EINVAL);
    EXPECT_EQ(open_fd_count(), before);
}
