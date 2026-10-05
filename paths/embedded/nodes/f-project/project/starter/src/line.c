#include "line.h"
#include "out.h"

void line_init(line_t *l) {
    (void)l;        /* TODO(m2) */
}

line_event_t line_feed(line_t *l, char c) {
    (void)l, (void)c;
    return LINE_NONE;   /* TODO(m2) */
}

const char *line_get(const line_t *l) {
    (void)l;
    return "";      /* TODO(m2) */
}
