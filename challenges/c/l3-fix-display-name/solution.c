#include <stddef.h>

void copy_display_name(char *dst, const char *src, size_t cap) {
    size_t i = 0;
    while (src[i] != '\0' && i + 1 < cap) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}
