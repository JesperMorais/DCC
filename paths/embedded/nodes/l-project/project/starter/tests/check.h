/* check.h: a tiny TAP-style test helper. No framework needed.
 *
 *   static void my_test(void) { CHECK(1 + 1 == 2); CHECKM(x > 0, "x was %d", x); }
 *   RUN(my_test);          prints "ok 1 - my_test" or "not ok 1 - my_test"
 *   return check_summary(); prints "# pass N" and "# fail N"
 *
 * CHECK and CHECKM are expressions that return whether the check held, so a
 * test can bail out early:  if (!CHECK(p != NULL)) return;
 */
#ifndef CHECK_H
#define CHECK_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

extern int check_pass_, check_fail_, check_num_;
extern bool check_ok_;

static inline bool check_(bool ok, const char *file, int line, const char *expr, const char *fmt, ...) {
    if (ok) return true;
    check_ok_ = false;
    printf("#   %s:%d: failed: %s\n", file, line, expr);
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        printf("#   ");
        vprintf(fmt, ap);
        printf("\n");
        va_end(ap);
    }
    fflush(stdout);
    return false;
}

#define CHECK(cond) check_((cond), __FILE__, __LINE__, #cond, NULL)
#define CHECKM(cond, ...) check_((cond), __FILE__, __LINE__, #cond, __VA_ARGS__)

#define RUN(fn)                                                              \
    do {                                                                     \
        check_ok_ = true;                                                    \
        test_begin();                                                        \
        fn();                                                                \
        test_end();                                                          \
        check_num_++;                                                        \
        if (check_ok_) check_pass_++; else check_fail_++;                    \
        printf("%s %d - %s\n", check_ok_ ? "ok" : "not ok", check_num_, #fn); \
        fflush(stdout);                                                      \
    } while (0)

static inline int check_summary(void) {
    printf("1..%d\n# pass %d\n# fail %d\n", check_num_, check_pass_, check_fail_);
    return check_fail_ ? 1 : 0;
}

/* Called around every test by RUN (defined in harness.c). */
void test_begin(void);
void test_end(void);

#endif
