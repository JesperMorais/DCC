/* The harness's runner and main (given). ./build/tests runs everything,
 * ./build/tests m2 runs one milestone. */
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "check.h"

void suite_m1(void);
void suite_m2(void);
void suite_m3(void);
void suite_m4(void);

static int n_run, n_pass, n_fail;
static int failed;   /* in the child: how many checks failed? */

void check_fail(const char *file, int line, const char *fmt, ...) {
    va_list ap;
    if (++failed > 8) {   /* the first few say it all */
        if (failed == 9) printf("#   (more failures in this test not shown)\n");
        return;
    }
    printf("#   %s:%d: ", file, line);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
}

void check_show(const char *label, const char *s) {
    if (failed > 8) return;
    printf("#     %-9s ", label);
    if (!s) {
        printf("NULL\n");
        return;
    }
    putchar('"');
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '\r') printf("\\r");
        else if (c == '\n') printf("\\n");
        else if (c == '\b') printf("\\b");
        else if (c == '\t') printf("\\t");
        else if (c == '"' || c == '\\') printf("\\%c", c);
        else if (c < 0x20 || c >= 0x7F) printf("\\x%02X", c);
        else putchar(c);
    }
    printf("\"\n");
}

void check_run(void (*fn)(void), const char *name) {
    n_run++;
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        alarm(2);
        fn();
        fflush(stdout);
        _exit(failed ? 3 : 0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        n_pass++;
        printf("ok %d - %s\n", n_run, name);
        return;
    }
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM)
        printf("#   timed out after 2 s: an endless loop?\n");
    else if (WIFSIGNALED(status))
        printf("#   crashed: %s\n", strsignal(WTERMSIG(status)));
    else if (WEXITSTATUS(status) != 3)
        printf("#   exited with status %d: see the sanitizer report above\n", WEXITSTATUS(status));
    n_fail++;
    printf("not ok %d - %s\n", n_run, name);
}

int main(int argc, char *argv[]) {
    const char *only = argc > 1 ? argv[1] : NULL;
    if (!only || strcmp(only, "m1") == 0) suite_m1();
    if (!only || strcmp(only, "m2") == 0) suite_m2();
    if (!only || strcmp(only, "m3") == 0) suite_m3();
    if (!only || strcmp(only, "m4") == 0) suite_m4();
    if (n_run == 0) {
        fprintf(stderr, "unknown milestone \"%s\" (try m1 to m4)\n", only);
        return 2;
    }
    printf("1..%d\n# pass %d\n# fail %d\n", n_run, n_pass, n_fail);
    return n_fail ? 1 : 0;
}
