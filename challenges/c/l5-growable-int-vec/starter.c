#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    int *data;    // heap block, or NULL while empty
    size_t len;   // how many values are stored
    size_t cap;   // how many values fit before we must grow
} IntVec;

void intvec_init(IntVec *v) {
    (void)v;
}

bool intvec_push(IntVec *v, int value) {
    (void)v;
    (void)value;
    return false;
}

int intvec_get(const IntVec *v, size_t index) {
    (void)v;
    (void)index;
    return 0;
}

void intvec_free(IntVec *v) {
    (void)v;
}
