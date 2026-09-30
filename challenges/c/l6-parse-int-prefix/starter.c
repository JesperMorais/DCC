#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

bool parse_int_prefix(const char *s, int *out, const char **end) {
    (void)out;
    *end = s;
    return false;
}
