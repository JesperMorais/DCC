### Two kinds of numbers

Every value in C has a **type**, and the type tells the computer how to store it and how to do maths with it.

- `int` holds **whole** numbers: `3`, `-12`, `2026`.
- `double` holds numbers **with decimals**: `3.5`, `-0.25`, `9.81`.

A function that works with decimals says so in its signature:

```c
double km_to_miles(double km) {
    return km * 0.621371;
}
```

The first `double` is the type of the answer. `double km` is the input, so `km_to_miles(10.0)` gives back `6.21371`.

### The integer-division trap

C picks the kind of maths from the types of the two values next to the operator:

```c
7 / 2      // 3    both are int, so the .5 is dropped
7.0 / 2    // 3.5  one is a double, so decimals are kept
7 / 2.0    // 3.5
```

So a bare `1 / 3` inside a decimal formula quietly becomes `0` and ruins the result. Writing the number with a `.0` makes it a `double`.

### Comparing decimals

Decimal numbers are stored in binary, so `0.1 + 0.2` is not *exactly* `0.3`. That's why the tests check that your answer is **very close** to the expected one (`EXPECT_NEAR`), rather than exactly equal.
