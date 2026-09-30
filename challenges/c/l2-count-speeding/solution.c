#include <stddef.h>

size_t count_speeding(const int *speeds, size_t count, int limit) {
    size_t speeding = 0;
    for (size_t i = 0; i < count; i++) {
        if (speeds[i] > limit) {
            speeding++;
        }
    }
    return speeding;
}
