#include <stdlib.h>
#include <string.h>

char *my_strdup(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t size = strlen(s) + 1;   // + 1 for the '\0'
    char *copy = malloc(size);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, s, size);
    return copy;
}
