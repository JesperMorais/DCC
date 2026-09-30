#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct Step {
    int action;          // an action id
    struct Step *next;   // the step before this one, or NULL
} Step;

bool history_push(Step **head, int action) {
    (void)head;
    (void)action;
    return false;
}

size_t history_length(const Step *head) {
    (void)head;
    return 0;
}

void history_free(Step *head) {
    (void)head;
}
