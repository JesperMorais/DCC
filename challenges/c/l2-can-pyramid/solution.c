int sum_one_to_n(int n) {
    int total = 0;
    for (int row = 1; row <= n; row++) {
        total += row;
    }
    return total;
}
