#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define LINE_CAP 64

typedef struct {
    char text[LINE_CAP];   // always NUL-terminated
    size_t len;
} EditLine;

void line_set(EditLine *line, const char *s) {
    size_t n = strlen(s);
    if (n > LINE_CAP - 1) {
        n = LINE_CAP - 1;
    }
    memcpy(line->text, s, n);
    line->text[n] = '\0';
    line->len = n;
}

bool line_insert(EditLine *line, size_t pos, const char *s) {
    size_t n = strlen(s);
    if (pos > line->len || line->len + n > LINE_CAP - 1) {
        return false;
    }
    for (size_t i = pos; i <= line->len; i++) {
        line->text[i + n] = line->text[i];
    }
    memcpy(line->text + pos, s, n);
    line->len += n;
    return true;
}

bool line_erase(EditLine *line, size_t pos, size_t count) {
    if (pos > line->len) {
        return false;
    }
    if (count > line->len - pos) {
        count = line->len - pos;
    }
    for (size_t i = pos + count; i <= line->len; i++) {
        line->text[i - count] = line->text[i];
    }
    line->len -= count;
    return true;
}

bool line_replace(EditLine *line, size_t pos, size_t count, const char *s) {
    if (pos > line->len) {
        return false;
    }
    if (count > line->len - pos) {
        count = line->len - pos;
    }
    if (line->len - count + strlen(s) > LINE_CAP - 1) {
        return false;
    }
    line_erase(line, pos, count);
    return line_insert(line, pos, s);
}
