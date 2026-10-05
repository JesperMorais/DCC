#include "stream.h"

#include <ctype.h>
#include <stdlib.h>

#include "util.h"

/* [+-]digits[.digits], nothing else. */
static bool is_number(const char *s) {
    if (*s == '+' || *s == '-') s++;
    if (!isdigit((unsigned char)*s)) return false;
    while (isdigit((unsigned char)*s)) s++;
    if (*s == '.') {
        s++;
        if (!isdigit((unsigned char)*s)) return false;
        while (isdigit((unsigned char)*s)) s++;
    }
    return *s == '\0';
}

static void add_sample(struct stream *s, double v) {
    struct window *w = &s->win;
    if (w->count == 0 || v < w->min) w->min = v;
    if (w->count == 0 || v > w->max) w->max = v;
    w->sum += v;
    w->count++;
    s->samples++;
}

static void end_line(struct stream *s) {
    s->line[s->len] = '\0';
    char *text = trim(s->line);
    if (s->too_long || !is_number(text)) s->errors++;
    else add_sample(s, strtod(text, NULL));
    s->len = 0;
    s->too_long = false;
}

void stream_feed(struct stream *s, const char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf[i] == '\n') end_line(s);
        else if (s->len < LINE_MAX_CHARS) s->line[s->len++] = buf[i];
        else s->too_long = true;
    }
}

void stream_finish(struct stream *s) {
    if (s->len > 0 || s->too_long) end_line(s);
}

void window_reset(struct window *w) {
    *w = (struct window){ 0 };
}
