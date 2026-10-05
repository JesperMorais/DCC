/* main.c: the test runner. "./build/run-tests" runs every milestone,
 * "./build/run-tests m2" just one. */
#include <stdio.h>
#include <string.h>

#include "check.h"
#include "run.h"

int check_count, check_passed, check_failed;

static const struct {
    const char *id;
    void (*fn)(void);
} milestones[] = { { "m1", m1_tests }, { "m2", m2_tests }, { "m3", m3_tests }, { "m4", m4_tests } };

int main(int argc, char **argv)
{
    const char *only = argc > 1 ? argv[1] : NULL;
    int ran = 0;
    for (size_t i = 0; i < sizeof milestones / sizeof milestones[0]; i++) {
        if (only && strcmp(only, milestones[i].id) != 0) continue;
        printf("# %s\n", milestones[i].id);
        fflush(stdout);
        milestones[i].fn();
        ran++;
    }
    if (!ran) {
        fprintf(stderr, "unknown milestone \"%s\"\n", only);
        return 2;
    }
    printf("1..%d\n# pass %d\n# fail %d\n", check_count, check_passed, check_failed);
    return check_failed ? 1 : 0;
}
