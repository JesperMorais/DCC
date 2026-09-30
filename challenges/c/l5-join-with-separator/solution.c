#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *join_strings(const char *const *parts, size_t count, const char *sep) {
    size_t sep_len = strlen(sep);
    size_t total = 1;   // the '\0'
    for (size_t i = 0; i < count; i++) {
        total += strlen(parts[i]);
        if (i > 0) {
            total += sep_len;
        }
    }

    char *out = malloc(total);
    if (out == NULL) {
        return NULL;
    }
    char *p = out;
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            memcpy(p, sep, sep_len);
            p += sep_len;
        }
        size_t len = strlen(parts[i]);
        memcpy(p, parts[i], len);
        p += len;
    }
    *p = '\0';
    return out;
}
