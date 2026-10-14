#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

void strbuf_init(StrBuf *sb) {
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

bool strbuf_append(StrBuf *sb, const char *text) {
    size_t n = strlen(text);
    if (sb->len + n + 1 > sb->cap) {
        size_t new_cap = sb->cap ? sb->cap * 2 : 16;
        char *grown = realloc(sb->data, new_cap);
        if (grown == NULL) {
            return false;
        }
        sb->data = grown;
        sb->cap = new_cap;
    }
    memcpy(sb->data + sb->len, text, n + 1);
    sb->len += n;
    return true;
}

void strbuf_free(StrBuf *sb) {
    free(sb->data);
    strbuf_init(sb);
}
