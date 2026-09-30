#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

bool parse_int_prefix(const char *s, int *out, const char **end) {
    const char *p = s;
    bool negative = false;
    if (*p == '+' || *p == '-') {
        negative = *p == '-';
        p++;
    }

    // Accumulate as a negative number: [INT_MIN, 0] has room for every valid magnitude.
    const char *digits = p;
    int value = 0;
    while (*p >= '0' && *p <= '9') {
        int d = *p - '0';
        if (value < (INT_MIN + d) / 10) {
            *end = s;   // too far below INT_MIN
            return false;
        }
        value = value * 10 - d;
        p++;
    }

    if (p == digits || (!negative && value == INT_MIN)) {
        *end = s;       // no digits, or +2147483648
        return false;
    }
    *out = negative ? value : -value;
    *end = p;
    return true;
}
