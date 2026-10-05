/* Milestone 1: the kernel boots, the sensor samples every 100 ms, the run ends cleanly. */
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "run.h"

#define RUN "--run-ms 2000"

void m1_tests(void)
{
    const run_t *r = plant(RUN);
    const line_t *last = last_line(r);

    CHECK(r->finished && r->exit_code == 0, "the run ends by itself with exit status 0", "%s", describe(r));

    CHECK(last && strcmp(last->tag, "END") == 0 && last->tick >= 2000 && last->tick <= 2050,
          "the last line is END, logged at tick 2000 (--run-ms)", "last line: %s %s",
          last ? "tick" : "(no output)", last ? last->text : "");

    CHECK(r->n > 0 && r->well_formed, "every line is \"<tick> <TAG> ...\"", "%s",
          r->n ? r->bad_line : describe(r));

    CHECK(r->hw[0] != '\0' && r->n > 0, "the board printed its report (the program ended via exit or return)",
          "no \"hw: ...\" line on stderr; %s", describe(r));

    const line_t *s[64];
    int n = find(r, "SENSOR", s, 64);
    CHECK(n >= 19 && n <= 20, "the sensor samples every 100 ms (19 or 20 SENSOR lines in 2000 ms)",
          "%d SENSOR lines", n);

    bool spaced = n >= 2;
    int bad = 0;
    for (int i = 1; i < n; i++)
        if (labs(s[i]->tick - s[i - 1]->tick - 100) > 2) {
            spaced = false;
            bad = i;
            break;
        }
    CHECK(spaced, "consecutive SENSOR lines are 100 ticks apart",
          n < 2 ? "fewer than 2 SENSOR lines" : "\"%ld %s\" then \"%ld %s\"", n < 2 ? 0L : s[bad ? bad - 1 : 0]->tick,
          n < 2 ? "" : s[bad ? bad - 1 : 0]->text, n < 2 ? 0L : s[bad]->tick, n < 2 ? "" : s[bad]->text);

    long drift = n >= 2 ? (s[n - 1]->tick - s[0]->tick) - 100L * (n - 1) : 999;
    long pmin = hw(r, "adc_period_min", -1), pmax = hw(r, "adc_period_max", -1);
    CHECK(labs(drift) <= 3 && pmin >= 97 && pmax <= 103,
          "the samples don't drift: conversions start on the 100 ms grid however long they take",
          "drift over the run: %ld ticks; the board saw conversions start %ld..%ld ticks apart", drift, pmin, pmax);

    long first = n ? kv(s[0]->text, "level", -1) : -1;
    bool levels = n > 0 && first >= 495 && first <= 520;
    for (int i = 0; i < n; i++) levels = levels && kv(s[i]->text, "level", -1) >= 0;
    CHECK(levels, "SENSOR lines carry the measured level (level=<mm>, about 500 at the first sample)",
          n ? "first SENSOR line: %s" : "no SENSOR lines%s", n ? s[0]->text : "");
}
