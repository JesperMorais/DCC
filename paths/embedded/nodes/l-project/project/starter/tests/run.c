/* The test runner: `tests/run` runs every milestone, `tests/run m2` only one,
 * and `tests/run m2 reconnect` only the m2 tests whose name contains
 * "reconnect". Use `make test` / `make test-m2` (add `T=reconnect` for one
 * test), which rebuild sensord first.
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
#define N_MILESTONES (sizeof milestones / sizeof milestones[0])

static bool is_milestone(const char *arg) {
    for (size_t i = 0; i < N_MILESTONES; i++)
        if (strcmp(arg, milestones[i].name) == 0) return true;
    return false;
}

int main(int argc, char **argv) {
    harness_init();
    bool any_milestone = false;
    for (int a = 1; a < argc; a++) {
        if (is_milestone(argv[a])) any_milestone = true;
        else check_only_ = argv[a];
    }
    for (size_t i = 0; i < N_MILESTONES; i++) {
        bool wanted = !any_milestone;
        for (int a = 1; a < argc; a++)
            if (strcmp(argv[a], milestones[i].name) == 0) wanted = true;
        if (!wanted) continue;
        printf("# %s\n", milestones[i].name);
        milestones[i].run();
    }
    if (check_only_ && check_num_ == 0) {
        printf("# no test name contains \"%s\"\n", check_only_);
        return 1;
    }
    return check_summary();
}
