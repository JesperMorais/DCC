/* A tiny test harness (given). Each test runs in its own process, so a crash
 * or an endless loop fails that one test and the rest still run. Output is
 * TAP-style: "ok N - name" / "not ok N - name", then "# pass N" and "# fail N". */
#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include <string.h>

void check_run(void (*fn)(void), const char *name);
void check_fail(const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void check_show(const char *label, const char *s);   /* prints s with \r, \n, \b... made visible */

#define RUN(fn, name) check_run(fn, name)

#define CHECK(cond) \
    do { if (!(cond)) check_fail(__FILE__, __LINE__, "CHECK(%s) failed", #cond); } while (0)

#define CHECK_INT(got, want) \
    do { long long g_ = (long long)(got), w_ = (long long)(want); \
         if (g_ != w_) check_fail(__FILE__, __LINE__, "%s is %lld, expected %lld", #got, g_, w_); } while (0)

/* Like CHECK_INT, but prints enum values by name: names[] lists them in order. */
#define CHECK_ENUM(got, want, names) CHECK_ENUM_(got, want, names, #got)
#define CHECK_ENUM_(got, want, names, text) \
    do { long long g_ = (long long)(got), w_ = (long long)(want); \
         long long n_ = (long long)(sizeof(names) / sizeof(names[0])); \
         if (g_ != w_) check_fail(__FILE__, __LINE__, "%s is %s (%lld), expected %s (%lld)", text, \
                                  g_ >= 0 && g_ < n_ ? names[g_] : "?", g_, \
                                  w_ >= 0 && w_ < n_ ? names[w_] : "?", w_); } while (0)

#define CHECK_HEX(got, want) \
    do { unsigned long g_ = (unsigned long)(got), w_ = (unsigned long)(want); \
         if (g_ != w_) check_fail(__FILE__, __LINE__, "%s is 0x%08lX, expected 0x%08lX", #got, g_, w_); } while (0)

#define CHECK_STR(got, want) \
    do { const char *g_ = (got), *w_ = (want); \
         if (!g_ || strcmp(g_, w_) != 0) { \
             check_fail(__FILE__, __LINE__, "%s is not what we expected", #got); \
             check_show("got", g_); check_show("expected", w_); } } while (0)

#define CHECK_PREFIX(got, prefix) \
    do { const char *g_ = (got), *p_ = (prefix); \
         if (!g_ || strncmp(g_, p_, strlen(p_)) != 0) { \
             check_fail(__FILE__, __LINE__, "%s should start with the prefix below", #got); \
             check_show("got", g_); check_show("prefix", p_); } } while (0)

#endif
