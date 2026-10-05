#include "line.h"
#include "out.h"

static int is_enter(char c) { return c == '\r' || c == '\n'; }
static int is_backspace(char c) { return c == '\b' || c == 0x7F; }
static int is_printable(char c) { return c >= 0x20 && c <= 0x7E; }

void line_init(line_t *l) {
    l->state = LINE_COLLECTING;
    l->len = 0;
    l->buf[0] = '\0';
}

/* Ends the current line: what comes next is a new one. */
static void finish(line_t *l, char c) {
    l->buf[l->len] = '\0';
    l->len = 0;
    l->state = c == '\r' ? LINE_AFTER_CR : LINE_COLLECTING;
    out_puts("\r\n");
}

line_event_t line_feed(line_t *l, char c) {
    switch (l->state) {
    case LINE_AFTER_CR:
        l->state = LINE_COLLECTING;
        if (c == '\n') return LINE_NONE;   /* the second half of \r\n */
        __attribute__((fallthrough));
    case LINE_COLLECTING:
        if (is_enter(c)) {
            finish(l, c);
            return LINE_READY;
        }
        if (is_backspace(c)) {
            if (l->len > 0) {
                l->len--;
                out_puts("\b \b");
            }
            return LINE_NONE;
        }
        if (!is_printable(c)) return LINE_NONE;
        if (l->len == LINE_MAX) {
            l->state = LINE_OVERFLOW;
            return LINE_NONE;
        }
        l->buf[l->len++] = c;
        out_putc(c);
        return LINE_NONE;
    case LINE_OVERFLOW:
        if (!is_enter(c)) return LINE_NONE;
        l->len = 0;
        finish(l, c);
        return LINE_TOO_LONG;
    }
    return LINE_NONE;
}

const char *line_get(const line_t *l) {
    return l->buf;
}
