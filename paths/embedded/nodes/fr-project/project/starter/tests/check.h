/* check.h: a tiny TAP-style test helper. No framework needed.
 *
 *   CHECK(cond, "name", "diagnostic printf format", args...)
 *
 * prints "ok N - name" or "not ok N - name" followed by "#   diagnostic".
 * With a filter (./build/run-tests m1 drift), only checks whose name contains
 * the filter text run. */
#ifndef CHECK_H
#define CHECK_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern int check_count, check_passed, check_failed;
extern const char *check_filter;

#define CHECK(cond, name, ...)                                  \
    do {                                                        \
        if (check_filter && !strstr(name, check_filter)) break; \
        check_count++;                                          \
        if (cond) {                                             \
            check_passed++;                                     \
            printf("ok %d - %s\n", check_count, name);          \
        } else {                                                \
            check_failed++;                                     \
            printf("not ok %d - %s\n#   ", check_count, name);  \
            printf(__VA_ARGS__);                                \
            printf("\n");                                       \
        }                                                       \
        fflush(stdout);                                         \
    } while (0)

#endif /* CHECK_H */
