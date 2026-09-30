#include <stddef.h>

double average_reading(const int *readings, size_t count) {
    if (count == 0) {
        return 0.0;
    }
    int sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += readings[i];
    }
    return (double)sum / (double)count;
}
