#include <stdbool.h>
#include <stddef.h>

bool next_word(const char **cursor, const char **word, size_t *len) {
    const char *p = *cursor;
    while (*p == ' ') {
        p++;
    }
    if (*p == '\0') {
        *cursor = p;
        return false;
    }
    const char *start = p;
    while (*p != '\0' && *p != ' ') {
        p++;
    }
    *word = start;
    *len = (size_t)(p - start);
    *cursor = p;
    return true;
}
