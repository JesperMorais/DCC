#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct Step {
    int action;          // an action id
    struct Step *next;   // the step before this one, or NULL
} Step;

bool history_push(Step **head, int action) {
    Step *node = malloc(sizeof *node);
    if (node == NULL) {
        return false;
    }
    node->action = action;
    node->next = *head;
    *head = node;
    return true;
}

size_t history_length(const Step *head) {
    size_t count = 0;
    for (const Step *s = head; s != NULL; s = s->next) {
        count++;
    }
    return count;
}

void history_free(Step *head) {
    while (head != NULL) {
        Step *next = head->next;   // read it before the node is gone
        free(head);
        head = next;
    }
}
