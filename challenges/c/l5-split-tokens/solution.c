#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void free_tokens(char **tokens) {
    if (tokens == NULL) {
        return;
    }
    for (size_t i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

static size_t count_tokens(const char *line, char delim) {
    size_t n = 0;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] != delim && (i == 0 || line[i - 1] == delim)) {
            n++;
        }
    }
    return n;
}

char **split_tokens(const char *line, char delim, size_t *count) {
    size_t n = count_tokens(line, delim);
    char **tokens = malloc((n + 1) * sizeof *tokens);   // + 1 for the NULL sentinel
    if (tokens == NULL) {
        return NULL;
    }

    const char *p = line;
    for (size_t i = 0; i < n; i++) {
        while (*p == delim) p++;               // skip to the token's start
        size_t len = 0;
        while (p[len] != '\0' && p[len] != delim) len++;

        tokens[i] = malloc(len + 1);
        if (tokens[i] == NULL) {
            free_tokens(tokens);               // tokens[i] is NULL: frees 0..i-1
            return NULL;
        }
        memcpy(tokens[i], p, len);
        tokens[i][len] = '\0';
        p += len;
    }
    tokens[n] = NULL;
    *count = n;
    return tokens;
}
