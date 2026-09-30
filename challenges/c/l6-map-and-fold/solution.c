#include <stddef.h>

void map_ints(int *arr, size_t n, int (*f)(int)) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = f(arr[i]);
    }
}

int fold_ints(const int *arr, size_t n, int init, int (*combine)(int, int)) {
    int acc = init;
    for (size_t i = 0; i < n; i++) {
        acc = combine(acc, arr[i]);
    }
    return acc;
}
