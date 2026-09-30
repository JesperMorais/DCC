#include <stddef.h>

size_t count_rises(const int *temps, size_t count) {
    size_t rises = 0;
    for (size_t i = 1; i < count; i++) {
        if (temps[i] > temps[i - 1]) {
            rises++;
        }
    }
    return rises;
}
