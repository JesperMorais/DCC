long sum_above(const int *begin, const int *end, int limit) {
    long total = 0;
    for (const int *p = begin; p != end; p++) {
        if (*p > limit) {
            total += *p;
        }
    }
    return total;
}
