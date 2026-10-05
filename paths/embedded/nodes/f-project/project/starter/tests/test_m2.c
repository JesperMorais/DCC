/* Milestone 2: the line editor. */
#include "check.h"
#include "line.h"
#include "sim.h"

/* Feeds every character of s. All but the last must give LINE_NONE; returns the last event. */
static line_event_t feed(line_t *l, const char *s, size_t n) {
    line_event_t ev = LINE_NONE;
    for (size_t i = 0; i < n; i++) {
        ev = line_feed(l, s[i]);
        if (i + 1 < n && ev != LINE_NONE) check_fail(__FILE__, __LINE__, "character %zu of the input gave an event too early", i);
    }
    return ev;
}
#define FEED(l, lit) feed(l, lit, sizeof(lit) - 1)

static void fresh(line_t *l) {
    sim_reset();
    line_init(l);
}

static void ready_on_cr(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "led on\r"), LINE_READY);
    CHECK_STR(line_get(&l), "led on");
}

static void echoes_what_you_type(void) {
    line_t l;
    fresh(&l);
    FEED(&l, "led on\r");
    CHECK_STR(sim_tx(), "led on\r\n");
}

static void ready_on_lf(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "gpio get 3\n"), LINE_READY);
    CHECK_STR(line_get(&l), "gpio get 3");
    CHECK_STR(sim_tx(), "gpio get 3\r\n");
}

static void crlf_is_one_enter(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "a\r"), LINE_READY);
    CHECK_STR(line_get(&l), "a");
    CHECK_INT(line_feed(&l, '\n'), LINE_NONE);
    CHECK_INT(FEED(&l, "b\r"), LINE_READY);
    CHECK_STR(line_get(&l), "b");
    CHECK_INT(FEED(&l, "\r"), LINE_READY);   /* but two CRs are two Enters */
    CHECK_STR(line_get(&l), "");
    CHECK_STR(sim_tx(), "a\r\nb\r\n\r\n");
}

static void backspace_edits(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "lex\x7f" "d\r"), LINE_READY);
    CHECK_STR(line_get(&l), "led");
    CHECK_STR(sim_tx(), "lex\b \bd\r\n");
    sim_tx_clear();
    CHECK_INT(FEED(&l, "ab\bc\r"), LINE_READY);   /* '\b' works too */
    CHECK_STR(line_get(&l), "ac");
}

static void backspace_on_empty_line(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "\x7f\x7fhi\r"), LINE_READY);
    CHECK_STR(line_get(&l), "hi");
    CHECK_STR(sim_tx(), "hi\r\n");
}

static void ignores_other_control_bytes(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "\tl\x1b" "e\x01\xc3" "d\r"), LINE_READY);
    CHECK_STR(line_get(&l), "led");
    CHECK_STR(sim_tx(), "led\r\n");
}

static void accepts_line_max(void) {
    line_t l;
    char in[LINE_MAX + 2], want[LINE_MAX + 1];
    fresh(&l);
    memset(in, 'x', LINE_MAX);
    in[LINE_MAX] = '\r';
    memset(want, 'x', LINE_MAX);
    want[LINE_MAX] = '\0';
    CHECK_INT(feed(&l, in, LINE_MAX + 1), LINE_READY);
    CHECK_STR(line_get(&l), want);
}

static void too_long_is_thrown_away(void) {
    line_t l;
    char in[LINE_MAX + 5], echo[LINE_MAX + 3];
    fresh(&l);
    memset(in, 'x', LINE_MAX + 4);
    in[LINE_MAX + 4] = '\r';
    memset(echo, 'x', LINE_MAX);
    memcpy(echo + LINE_MAX, "\r\n", 3);
    CHECK_INT(feed(&l, in, LINE_MAX + 5), LINE_TOO_LONG);
    CHECK_STR(sim_tx(), echo);   /* the extra characters aren't echoed */
    CHECK_INT(FEED(&l, "ok\r"), LINE_READY);
    CHECK_STR(line_get(&l), "ok");
}

static void backspace_doesnt_rescue_overflow(void) {
    line_t l;
    char in[LINE_MAX + 4];
    fresh(&l);
    memset(in, 'y', LINE_MAX + 1);
    in[LINE_MAX + 1] = 0x7f;
    in[LINE_MAX + 2] = 0x7f;
    in[LINE_MAX + 3] = '\n';
    CHECK_INT(feed(&l, in, LINE_MAX + 4), LINE_TOO_LONG);
}

static void empty_line_is_ready(void) {
    line_t l;
    fresh(&l);
    CHECK_INT(FEED(&l, "\r"), LINE_READY);
    CHECK_STR(line_get(&l), "");
    CHECK_STR(sim_tx(), "\r\n");
}

void suite_m2(void) {
    RUN(ready_on_cr, "m2: '\\r' finishes a line");
    RUN(echoes_what_you_type, "m2: typed characters are echoed, Enter echoes \\r\\n");
    RUN(ready_on_lf, "m2: '\\n' finishes a line too");
    RUN(crlf_is_one_enter, "m2: \\r\\n counts as one Enter");
    RUN(backspace_edits, "m2: backspace (0x7F or \\b) removes a character and echoes \\b \\b");
    RUN(backspace_on_empty_line, "m2: backspace on an empty line does nothing");
    RUN(ignores_other_control_bytes, "m2: other control bytes and non-ASCII are ignored");
    RUN(accepts_line_max, "m2: a line of exactly LINE_MAX characters is accepted");
    RUN(too_long_is_thrown_away, "m2: a longer line gives LINE_TOO_LONG and the next line works");
    RUN(backspace_doesnt_rescue_overflow, "m2: backspace can't undo an overflow");
    RUN(empty_line_is_ready, "m2: Enter on an empty line gives an empty line");
}
