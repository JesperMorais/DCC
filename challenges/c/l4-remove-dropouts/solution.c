#include <stddef.h>

size_t remove_sentinel(int *readings, size_t count, int sentinel) {
    if (readings == NULL) {
        return 0;
    }
    size_t w = 0;
    for (size_t r = 0; r < count; r++) {
        if (readings[r] != sentinel) {
            readings[w++] = readings[r];
        }
    }
    return w;
}
