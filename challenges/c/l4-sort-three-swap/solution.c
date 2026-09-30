void swap_ints(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void sort_three(int *a, int *b, int *c) {
    if (*a > *b) swap_ints(a, b);
    if (*b > *c) swap_ints(b, c);
    if (*a > *b) swap_ints(a, b);
}
