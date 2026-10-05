/* Milestone 4: a software timer gives a heartbeat and catches a stalled sensor. */
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "run.h"

#define HEALTHY "--run-ms 2000"
#define STALL_AT 1350
#define STALLED "--run-ms 2400 --sensor-stall-at 1350"

void m4_tests(void)
{
    const run_t *r = plant(HEALTHY);
    const line_t *h[32];
    int nh = find(r, "HEARTBEAT", h, 32);

    bool spaced = nh >= 7 && nh <= 8 && labs(h[0]->tick - 250) <= 2;
    for (int i = 1; i < nh; i++) spaced = spaced && labs(h[i]->tick - h[i - 1]->tick - 250) <= 2;
    for (int i = 0; i < nh; i++) spaced = spaced && labs(h[i]->tick - 250L * (i + 1)) <= 3;
    CHECK(spaced, "a HEARTBEAT line every 250 ticks", "%d HEARTBEAT lines%s%s", nh, nh ? ", first: " : "",
          nh ? h[0]->text : "");

    bool counts = nh > 0;
    long total = 0;
    for (int i = 0; i < nh; i++) {
        long c = kv(h[i]->text, "checkins", -1);
        counts = counts && c >= 2 && c <= 3;
        total += c;
    }
    CHECK(counts && labs(total - count(r, "SENSOR")) <= 1,
          "each HEARTBEAT reports checkins=<sensor cycles in that window> (2 or 3)",
          "checkins add up to %ld over %d SENSOR lines; first: %s", total, count(r, "SENSOR"),
          nh ? h[0]->text : "(none)");

    CHECK(r->finished && nh >= 7 && count(r, "WATCHDOG") == 0, "no WATCHDOG while the sensor is healthy", "%d WATCHDOG lines; %s",
          count(r, "WATCHDOG"), describe(r));

    const run_t *st = plant(STALLED);
    const line_t *w[4], *p[64];
    int nw = find(st, "WATCHDOG", w, 4);
    CHECK(st->finished && st->exit_code == 0 && nw == 1 && w[0]->tick > STALL_AT &&
              w[0]->tick <= STALL_AT + 2 * 250 + 5,
          "a stalled sensor trips the watchdog once, within two windows",
          "%d WATCHDOG lines%s%s (the sensor stalls at %d); %s", nw, nw ? ", first: " : "", nw ? w[0]->text : "",
          STALL_AT, describe(st));

    int np = find(st, "PUMP", p, 64);
    bool safe = nw == 1 && st->hw[0] && strstr(st->hw, "pump=off") && hw(st, "dry_run_ticks", -1) == 0;
    char word[16];
    for (int i = 0; i < np && nw; i++)
        if (p[i]->tick > w[0]->tick && strcmp(nth_word(p[i]->text, 1, word, sizeof word), "on") == 0) safe = false;
    CHECK(safe, "the trip switches the pump off for good, before it can run dry", "board report: %s",
          st->hw[0] ? st->hw : describe(st));

    int beats_after = 0;
    for (int i = 0; i < st->n && nw; i++)
        beats_after += strcmp(st->lines[i].tag, "HEARTBEAT") == 0 && st->lines[i].tick > w[0]->tick;
    CHECK(nw == 1 && beats_after == 0 && count(st, "HEARTBEAT") >= 4,
          "heartbeats until the stall, then the WATCHDOG line, then silence",
          "%d HEARTBEAT lines in total, %d after the WATCHDOG line", count(st, "HEARTBEAT"), beats_after);
}
