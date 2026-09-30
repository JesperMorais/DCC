#include <stdbool.h>
#include <stddef.h>

bool min_max(const int *values, size_t count, int *min, int *max) {
    if (values == NULL || count == 0) {
        return false;
    }
    *min = values[0];
    *max = values[0];
    for (size_t i = 1; i < count; i++) {
        if (values[i] < *min) *min = values[i];
        if (values[i] > *max) *max = values[i];
    }
    return true;
}
