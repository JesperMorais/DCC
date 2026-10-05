/* Milestone 2: readings flow through a queue to the control task, which drives the pump. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "run.h"

#define RUN "--run-ms 2950"

void m2_tests(void)
{
    const run_t *r = plant(RUN);
    const line_t *s[64], *p[64];
    int ns = find(r, "SENSOR", s, 64);
    int np = find(r, "PUMP", p, 64);
    char word[16];

    CHECK(r->finished && r->exit_code == 0 && hw(r, "overflow_ticks", -1) == 0 && hw(r, "dry_run_ticks", -1) == 0,
          "the tank never overflows and the pump never runs dry", "board report: %s",
          r->hw[0] ? r->hw : describe(r));

    /* Replay the hysteresis over the readings that were printed. */
    char expected[64][24], actual[64][24];
    int ne = 0;
    bool on = false;
    for (int i = 0; i < ns && ne < 64; i++) {
        long level = kv(s[i]->text, "level", -1);
        if (!on && level >= 700) on = true;
        else if (on && level <= 300) on = false;
        else continue;
        snprintf(expected[ne++], sizeof expected[0], "%s level=%ld", on ? "on" : "off", level);
    }
    for (int i = 0; i < np; i++)
        snprintf(actual[i], sizeof actual[0], "%s level=%ld", nth_word(p[i]->text, 1, word, sizeof word),
                 kv(p[i]->text, "level", -1));
    int first_diff = -1;
    for (int i = 0; i < (ne > np ? ne : np); i++)
        if (i >= ne || i >= np || strcmp(expected[i], actual[i]) != 0) {
            first_diff = i;
            break;
        }
    CHECK(ne >= 4 && first_diff < 0,
          "the pump switches on at >= 700 mm and off at <= 300 mm, and only then",
          "PUMP line %d: expected \"PUMP %s\", got \"PUMP %s\" (%d PUMP lines, %d expected)", first_diff + 1,
          first_diff >= 0 && first_diff < ne ? expected[first_diff] : "(none)",
          first_diff >= 0 && first_diff < np ? actual[first_diff] : "(none)", np, ne);

    /* Each PUMP line follows the reading that caused it within a few ticks. */
    bool prompt = np > 0;
    int late = -1;
    for (int i = 0; i < np && prompt; i++) {
        long level = kv(p[i]->text, "level", -1), sample_tick = -1;
        for (int j = 0; j < ns; j++)
            if (kv(s[j]->text, "level", -2) == level && s[j]->tick <= p[i]->tick) sample_tick = s[j]->tick;
        if (sample_tick < 0 || p[i]->tick - sample_tick > 10) {
            prompt = false;
            late = i;
        }
    }
    CHECK(prompt, "each PUMP line comes within 10 ticks of the sample that triggered it", "%s",
          late >= 0 ? p[late]->text : "no PUMP lines");

    const line_t *end = last_line(r);
    long samples = end ? kv(end->text, "samples", -1) : -1;
    long starts = end ? kv(end->text, "pump_starts", -1) : -1;
    long max_level = end ? kv(end->text, "max_level", -1) : -1;
    long seen_max = -1, on_lines = 0;
    for (int i = 0; i < ns; i++)
        if (kv(s[i]->text, "level", -1) > seen_max) seen_max = kv(s[i]->text, "level", -1);
    for (int i = 0; i < np; i++) on_lines += strcmp(nth_word(p[i]->text, 1, word, sizeof word), "on") == 0;

    CHECK(end && strcmp(end->tag, "END") == 0 && samples == ns,
          "END reports samples=<readings the control task handled>", "END line: \"%s\", %d SENSOR lines",
          end ? end->text : "(none)", ns);
    CHECK(starts == on_lines && starts == hw(r, "pump_starts", -2) && max_level == seen_max,
          "END reports pump_starts=<n> and max_level=<mm> from the shared status",
          "END line: \"%s\"; %ld PUMP on lines, the board counted %ld starts, highest SENSOR level %ld",
          end ? end->text : "(none)", on_lines, hw(r, "pump_starts", -1), seen_max);
}
