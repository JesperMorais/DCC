/* The test runner: `tests/run` runs every milestone, `tests/run m2` only one.
 * Use `make test` / `make test-m2`, which rebuild sensord first.
 */
#include <stdio.h>
#include <string.h>

#include "harness.h"

void m1_tests(void);
void m2_tests(void);
void m3_tests(void);
void m4_tests(void);
void m5_tests(void);

static const struct {
    const char *name;
    void (*run)(void);
} milestones[] = {
    { "m1", m1_tests }, { "m2", m2_tests }, { "m3", m3_tests }, { "m4", m4_tests }, { "m5", m5_tests },
};

int main(int argc, char **argv) {
    harness_init();
    for (size_t i = 0; i < sizeof milestones / sizeof milestones[0]; i++) {
        bool wanted = argc < 2;
        for (int a = 1; a < argc; a++)
            if (strcmp(argv[a], milestones[i].name) == 0) wanted = true;
        if (!wanted) continue;
        printf("# %s\n", milestones[i].name);
        milestones[i].run();
    }
    return check_summary();
}
