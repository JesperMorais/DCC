#include <stdbool.h>
#include <string.h>

bool split_header(char *line, char **name, char **value) {
    char *colon = strrchr(line, ':');
    if (colon == NULL || colon == line) {
        return false;
    }
    *colon = '\0';

    char *v = colon + 1;
    while (*v == ' ') {
        v++;
    }
    *name = line;
    *value = v;
    return true;
}
