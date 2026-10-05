/* Milestone 3: the e-stop interrupt wakes a task that stops the pump at once. */
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "run.h"

#define PRESS 1537
#define RUN "--run-ms 2000 --estop-at 1537"

void m3_tests(void)
{
    const run_t *r = plant(RUN);
    const line_t *e[8], *p[64], *s[64];
    int ne = find(r, "ESTOP", e, 8);
    int np = find(r, "PUMP", p, 64);
    int ns = find(r, "SENSOR", s, 64);
    char word[16];

    long irq = ne ? kv(e[0]->text, "irq", -1) : -1;
    CHECK(r->finished && r->exit_code == 0 && ne == 1 && labs(irq - PRESS) <= 1 && e[0]->tick - irq <= 2,
          "one ESTOP line, irq=<tick of the first edge>, logged within 2 ticks of it",
          "%d ESTOP lines%s%s; %s", ne, ne ? ", first: " : "", ne ? e[0]->text : "", describe(r));

    CHECK(hw(r, "estop_preempted", -1) == 1,
          "the woken task runs as soon as the ISR returns, not at the next tick",
          "the pump was still on when the e-stop ISR returned (estop_preempted=%ld). Board report: %s",
          hw(r, "estop_preempted", -1), r->hw[0] ? r->hw : "(none)");

    bool stays_off = ne == 1 && r->hw[0] && strstr(r->hw, "pump=off") && hw(r, "estop_pump_ticks", 99) <= 1;
    for (int i = 0; i < np; i++)
        if (p[i]->tick > PRESS && strcmp(nth_word(p[i]->text, 1, word, sizeof word), "on") == 0)
            stays_off = false;
    CHECK(stays_off, "the pump goes off at the press and stays off (the e-stop latches)", "board report: %s",
          r->hw[0] ? r->hw : describe(r));

    const line_t *end = last_line(r);
    long counted = end ? kv(end->text, "estop_irqs", -1) : -1;
    CHECK(counted == 3 && hw(r, "estop_irqs", -1) == 3,
          "END reports estop_irqs=3: every bouncing edge is counted, but only one ESTOP is logged",
          "END line: \"%s\"", end ? end->text : "(none)");

    int after = 0;
    for (int i = 0; i < ns; i++) after += s[i]->tick > PRESS + 50;
    CHECK(after >= 3, "the sensor keeps sampling after the e-stop", "%d SENSOR lines after tick %d", after,
          PRESS + 50);
}
