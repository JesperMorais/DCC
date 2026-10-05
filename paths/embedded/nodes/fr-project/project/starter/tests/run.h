/* run.h: runs ./build/plant with scenario options and parses what it printed.
 *
 * The tests never link against your code: they run the program, read its
 * stdout ("<tick> <TAG> key=value ...") and the board's "hw: ..." report on
 * stderr, and check behaviour. Any structure that behaves right passes. */
#ifndef RUN_H
#define RUN_H

#include <stdbool.h>

#define MAX_LINES 2000

typedef struct {
    long tick;
    char tag[16];
    char text[128]; /* the whole line after the tick, e.g. "PUMP on level=706" */
} line_t;

typedef struct {
    char args[128];
    bool finished;      /* exited by itself before the time limit */
    int exit_code;      /* -1 if killed or crashed */
    bool well_formed;   /* every stdout line was "<tick> <TAG>..." */
    char bad_line[160]; /* the first line that wasn't */
    int n;
    line_t lines[MAX_LINES];
    char hw[512];       /* the board's report, "" if it never printed one */
    char err_tail[512]; /* the last stderr output, for diagnostics */
} run_t;

/* Runs the plant once per distinct args string and caches the result. */
const run_t *plant(const char *args);

/* Lines with this tag, in printed order. Returns how many (at most max). */
int find(const run_t *r, const char *tag, const line_t **out, int max);
int count(const run_t *r, const char *tag);
const line_t *last_line(const run_t *r);

/* The integer after "key=" in text, or dflt if it isn't there. */
long kv(const char *text, const char *key, long dflt);
/* The n-th space-separated word of text (0 is the tag), copied into buf. */
const char *nth_word(const char *text, int n, char *buf, int size);
/* A value from the board's report, or dflt. */
long hw(const run_t *r, const char *key, long dflt);

/* One line summary of a run for diagnostics: how it ended and its stderr. */
const char *describe(const run_t *r);

void m1_tests(void);
void m2_tests(void);
void m3_tests(void);
void m4_tests(void);

#endif /* RUN_H */
