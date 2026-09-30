#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    int *data;    // heap block, or NULL while empty
    size_t len;   // how many values are stored
    size_t cap;   // how many values fit before we must grow
} IntVec;

void intvec_init(IntVec *v) {
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
}

bool intvec_push(IntVec *v, int value) {
    if (v->len == v->cap) {
        size_t new_cap = v->cap == 0 ? 4 : v->cap * 2;
        int *grown = realloc(v->data, new_cap * sizeof *grown);
        if (grown == NULL) {
            return false;   // v->data is still valid and unchanged
        }
        v->data = grown;
        v->cap = new_cap;
    }
    v->data[v->len++] = value;
    return true;
}

int intvec_get(const IntVec *v, size_t index) {
    return v->data[index];
}

void intvec_free(IntVec *v) {
    free(v->data);
    intvec_init(v);
}
