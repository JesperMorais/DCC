#include <stddef.h>

int total_steps(const int *steps, size_t count) {
    int total = 0;
    for (size_t i = 0; i < count; i++) {
        total += steps[i];
    }
    return total;
}
