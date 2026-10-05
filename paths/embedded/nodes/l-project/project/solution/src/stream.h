#ifndef STREAM_H
#define STREAM_H

#include <stdbool.h>
#include <stddef.h>

#define LINE_MAX_CHARS 63

/* Statistics over one period. */
struct window {
    int count;
    double min, max, sum;
};

/* Turns bytes from the sensor into samples. Bytes after the last '\n' wait
 * here for the rest of their line. */
struct stream {
    char line[LINE_MAX_CHARS + 1];
    size_t len;
    bool too_long;
    struct window win;
    unsigned long samples, errors;
};

void stream_feed(struct stream *s, const char *buf, size_t len);
/* At EOF or shutdown: an unfinished line is an error. */
void stream_finish(struct stream *s);
void window_reset(struct window *w);

#endif
