#include <stddef.h>

void map_ints(int *arr, size_t n, int (*f)(int)) {
    (void)arr;
    (void)n;
    (void)f;
}

int fold_ints(const int *arr, size_t n, int init, int (*combine)(int, int)) {
    (void)arr;
    (void)n;
    (void)combine;
    return init;
}
