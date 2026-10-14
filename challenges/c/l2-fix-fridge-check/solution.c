#include <stdbool.h>
#include <stddef.h>

bool all_readings_in_range(const int *readings, size_t count, int low, int high) {
    for (size_t i = 0; i < count; i++) {
        if (readings[i] < low || readings[i] > high) {
            return false;
        }
    }
    return true;
}
