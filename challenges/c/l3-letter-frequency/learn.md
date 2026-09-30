### Characters are numbers

In C, `'a'` is just a small integer (97 in ASCII), and the letters `a`–`z` are consecutive. That means subtracting `'a'` maps a lowercase letter to a position:

```c
'a' - 'a'   // 0
'd' - 'a'   // 3
'z' - 'a'   // 25
```

The same trick works for digits: `'7' - '0'` is `7`.

### An array as a tally

A fixed-size `int` array makes a great counter when the possible keys are small numbers. Here's a histogram of dice rolls, where the values 1–6 map to indices 0–5:

```c
void tally_rolls(const int *rolls, size_t n, int hist[6]) {
    for (int k = 0; k < 6; k++) hist[k] = 0;
    for (size_t i = 0; i < n; i++) {
        if (rolls[i] >= 1 && rolls[i] <= 6) hist[rolls[i] - 1]++;
    }
}
```

Two things to notice: it **zeroes the array first**, and it **checks the range** before using a value as an index. An index outside `0..5` would write past the end of `hist`.

`int hist[6]` in a parameter list is really just `int *hist`. The `6` documents what the caller must provide.
