int count_digits(int n) {
    int digits = 1;
    while (n / 10 != 0) {
        n /= 10;
        digits++;
    }
    return digits;
}
