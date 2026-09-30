#include <stddef.h>

const char *file_extension(const char *path) {
    const char *dot = NULL;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '.') {
            dot = p;
        }
    }
    return dot != NULL ? dot + 1 : NULL;
}
