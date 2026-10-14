#include <stddef.h>

int coldest_reading(const int *readings, size_t count) {
    int coldest = 0;
    for (size_t i = 0; i < count; i++) {
        if (readings[i] < coldest) {
            coldest = readings[i];
        }
    }
    return coldest;
}
