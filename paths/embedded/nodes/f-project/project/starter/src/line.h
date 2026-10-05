/* Milestone 2: the line editor, a small state machine fed one character at a time. */
#ifndef LINE_H
#define LINE_H

#include <stdint.h>

#define LINE_MAX 32   /* longest line accepted, not counting the terminating '\0' */

typedef enum {
    LINE_COLLECTING,  /* normal typing */
    LINE_OVERFLOW,    /* the line got too long: ignore everything until Enter */
    LINE_AFTER_CR,    /* the last character was '\r': a '\n' right now is part of the same Enter */
} line_state_t;

typedef struct {
    line_state_t state;
    char buf[LINE_MAX + 1];
    uint32_t len;
} line_t;

typedef enum {
    LINE_NONE,      /* nothing to do yet */
    LINE_READY,     /* a complete line is in line_get() */
    LINE_TOO_LONG,  /* a line was too long and has been thrown away */
} line_event_t;

void line_init(line_t *l);
line_event_t line_feed(line_t *l, char c);
const char *line_get(const line_t *l);   /* the finished line, valid until the next line_feed */

#endif
