An editor records every action in an undo history. The newest action sits at the **front**, so undo always takes the first node. Build the list with these three functions:

```c
typedef struct Step {
    int action;          // an action id
    struct Step *next;   // the step before this one, or NULL
} Step;

bool   history_push(Step **head, int action);
size_t history_length(const Step *head);
void   history_free(Step *head);
```

- `history_push` allocates a new node holding `action` and puts it at the **front** of the list. It returns `false` only if `malloc` fails.
- `history_length` counts the nodes. An empty history is `NULL` and has length 0.
- `history_free` frees **every** node. `history_free(NULL)` does nothing.

```c
Step *history = NULL;
history_push(&history, 1);
history_push(&history, 2);
history_push(&history, 3);   // list is now 3 → 2 → 1
history_length(history);     // → 3
history_free(history);
```

The tests check for leaks, so `history_free` really has to free everything.
