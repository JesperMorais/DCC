#include <stddef.h>

void reverse_into(const int *src, size_t count, int *out) {
    for (size_t i = 0; i < count; i++) {
        out[i] = src[count - 1 - i];
    }
}
