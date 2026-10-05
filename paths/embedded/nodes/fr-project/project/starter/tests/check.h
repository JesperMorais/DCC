/* check.h: a tiny TAP-style test helper. No framework needed.
 *
 *   CHECK(cond, "name", "diagnostic printf format", args...)
 *
 * prints "ok N - name" or "not ok N - name" followed by "#   diagnostic". */
#ifndef CHECK_H
#define CHECK_H

#include <stdbool.h>
#include <stdio.h>

extern int check_count, check_passed, check_failed;

#define CHECK(cond, name, ...)                                  \
    do {                                                        \
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
