#include <limits.h>
#include <stdbool.h>

bool checked_add(int a, int b, int *out) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
        return false;
    }
    *out = a + b;
    return true;
}

bool checked_sub(int a, int b, int *out) {
    if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b)) {
        return false;
    }
    *out = a - b;
    return true;
}
