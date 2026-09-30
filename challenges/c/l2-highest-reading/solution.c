#include <stddef.h>

int highest_reading(const int *values, size_t count) {
    int highest = values[0];
    for (size_t i = 1; i < count; i++) {
        if (values[i] > highest) {
            highest = values[i];
        }
    }
    return highest;
}
