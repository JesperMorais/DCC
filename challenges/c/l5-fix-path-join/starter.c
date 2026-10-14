#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char *path_join(const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    bool need_slash = dir_len > 0 && dir[dir_len - 1] != '/';

    char *path = malloc(dir_len + name_len + 1);
    if (path == NULL) {
        return NULL;
    }
    memcpy(path, dir, dir_len);
    size_t pos = dir_len;
    if (need_slash) {
        path[pos++] = '/';
    }
    memcpy(path + pos, name, name_len + 1);
    return path;
}
